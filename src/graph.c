#include <stdint.h>
#include <string.h>

#include <jive/graph.h>
#include <jive/nodeclass.h>
#include <jive/internal/graphstr.h>
#include "debug.h"

#include "compiler.h"

/* private helper functions */

#define PUSH_BACK(list, edge, ANCHOR) \
do {\
	if ((list)->last) (list)->last->ANCHOR.next=edge;\
	else (list)->first=edge;\
	edge->ANCHOR.prev=(list)->last;\
	edge->ANCHOR.next=0;\
	(list)->last=edge;\
} while(0)

#define PUSH_FRONT(list, edge, ANCHOR) \
do {\
	if ((list)->first) (list)->first->ANCHOR.prev=edge;\
	else (list)->last=edge;\
	edge->ANCHOR.prev=0;\
	edge->ANCHOR.next=(list)->first;\
	(list)->first=edge;\
} while(0)

#define REMOVE(list, edge, ANCHOR) \
do {\
	if (edge->ANCHOR.prev) edge->ANCHOR.prev->ANCHOR.next=edge->ANCHOR.next;\
	else (list)->first=edge->ANCHOR.next;\
	if (edge->ANCHOR.next) edge->ANCHOR.next->ANCHOR.prev=edge->ANCHOR.prev;\
	else (list)->last=edge->ANCHOR.prev;\
} while(0)

static inline void
jive_edgelist_init(jive_edgelist * list)
{
	list->first = list->last = 0;
}

static inline jive_edge_container *
jive_graph_edge_allocate(jive_graph * graph)
{
	jive_edge_container * edge;
	edge = graph->unused.first;
	if (edge) {
		REMOVE(&graph->unused, edge, input_list);
		return edge;
	}
	edge=(jive_edge_container *)jive_context_malloc(graph->context, sizeof(*edge));
	
	return edge;
}

static inline void
jive_graph_edge_unused(jive_graph * graph, jive_edge_container * edge)
{
	PUSH_FRONT(&graph->unused, edge, input_list);
}

static inline void
mark_dangling_node_input(jive_graph * graph, jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	jive_edge_container * edge = jive_graph_edge_allocate(graph);
	
	edge->base.origin.node = 0;
	edge->base.origin.port = 0;
	edge->base.target.node = node;
	edge->base.target.port = 0;
	
	PUSH_FRONT(&head->input_edges, edge, input_list);
	PUSH_FRONT(&graph->null_origin, edge, output_list);
}

static inline void
mark_dangling_node_output(jive_graph * graph, jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	jive_edge_container * edge = jive_graph_edge_allocate(graph);
	
	edge->base.origin.node = node;
	edge->base.origin.port = 0;
	edge->base.target.node = 0;
	edge->base.target.port = 0;
	
	PUSH_FRONT(&graph->null_target, edge, input_list);
	PUSH_FRONT(&head->output_edges, edge, output_list);
}

static inline void
mark_node_input_connected(jive_graph * graph, jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	jive_edge_container * edge=head->input_edges.first;
	if (!edge->base.origin.node) {
		REMOVE(&head->input_edges, edge, input_list);
		REMOVE(&graph->null_origin, edge, output_list);
		jive_graph_edge_unused(graph, edge);
	}
}

static inline void
mark_node_output_connected(jive_graph * graph, jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	jive_edge_container * edge=head->output_edges.first;
	if (!edge->base.target.node) {
		REMOVE(&head->output_edges, edge, output_list);
		REMOVE(&graph->null_target, edge, input_list);
		jive_graph_edge_unused(graph, edge);
	}
}

static inline bool
all_inputs_behind_cut(jive_graph_traverser * traverser, jive_node * node)
{
	jive_input_edge_iterator iterator;
	JIVE_ITERATE_INPUTS(iterator, node)
		if (!node_behind_cut(traverser, iterator->origin.node)) return false;
	return true;
}

static inline bool
all_outputs_behind_cut(jive_graph_traverser * traverser, jive_node * node)
{
	jive_output_edge_iterator iterator;
	JIVE_ITERATE_OUTPUTS(iterator, node)
		if (!node_behind_cut(traverser, iterator->target.node)) return false;
	return true;
}

static inline void
add_to_cut(jive_graph_traverser * traverser, jive_node * node)
{
	jive_node_traversal_state * state = traversal_state(traverser, node);
	
	if (node_in_cut(traverser, node)) return;
	
	state->cookie = traverser->cookie-1;
	
	if (traverser->cut.last) traversal_state(traverser, traverser->cut.last)->next = node;
	else traverser->cut.first = node;
	
	state->prev = traverser->cut.last;
	state->next = 0;
	
	traverser->cut.last = node;
}

static inline void
remove_from_cut(jive_graph_traverser * traverser, jive_node * node)
{
	jive_node_traversal_state * state = traversal_state(traverser, node);
	
	if (state->next) traversal_state(traverser, state->next)->prev = state->prev;
	else traverser->cut.last = state->prev;
	if (state->prev) traversal_state(traverser, state->prev)->next = state->next;
	else traverser->cut.first = state->next;
	
	state->prev = state->next = 0;
}

static inline void
remove_from_cuts(jive_node * node)
{
	jive_graph_traverser * trav;
	jive_graph * graph = node->graph;
	JIVE_ITERATE_ACTIVE_TRAVERSERS(graph, trav) {
		if (node_in_cut(trav, node)) remove_from_cut(trav, node);
	}
}

/* if this node has an input edge added or removed, revalidate
its state wrt to concurrent traversers (it may become "visitable"
if an edge was removed, or it may become "unvisitable" if
an edge was added */
static inline void
check_topdown_traversers(jive_node * node)
{
	jive_graph * graph = node->graph;
	jive_graph_traverser * trav;
	JIVE_ITERATE_ACTIVE_TRAVERSERS(graph, trav) {
		if (trav->direction != +1) continue;
		
		if (all_inputs_behind_cut(trav, node)) {
			if (node_before_cut(trav, node)) add_to_cut(trav, node);
		} else {
			if (node_in_cut(trav, node)) remove_from_cut(trav, node);
			mark_node_before_cut(trav, node);
		}
	}
}

/* if this node has an output edge added or removed, revalidate
its state wrt to concurrent traversers (it may become "visitable"
if an edge was removed, or it may become "unvisitable" if
an edge was added */
static inline void
check_bottomup_traversers(jive_node * node)
{
	jive_graph * graph = node->graph;
	jive_graph_traverser * trav;
	JIVE_ITERATE_ACTIVE_TRAVERSERS(graph, trav) {
		if (trav->direction != -1) continue;
		
		if (all_outputs_behind_cut(trav, node)) {
			if (node_before_cut(trav, node)) add_to_cut(trav, node);
		} else {
			if (node_in_cut(trav, node)) remove_from_cut(trav, node);
			mark_node_before_cut(trav, node);
		}
	}
}

static inline void
jive_edge_remove(jive_edge * _edge)
{
	jive_edge_container * edge = (jive_edge_container *)_edge;
	jive_node * target = edge->base.target.node, * origin = edge->base.origin.node;
	REMOVE(&head_of_node(target)->input_edges, edge, input_list);
	REMOVE(&head_of_node(origin)->output_edges, edge, output_list);
	jive_graph_edge_unused(target->graph, edge);
	
	if (!head_of_node(origin)->output_edges.first) mark_dangling_node_output(origin->graph, origin);
	if (!head_of_node(target)->input_edges.first) mark_dangling_node_input(target->graph, target);
	
	check_topdown_traversers(target);
	check_bottomup_traversers(origin);
}

static bool
_jive_node_prune_recursive(jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	jive_edge_container * edge;
	
	if (head->output_edges.first->base.target.node) return false;
	if (head->reservation) return false;
	
	remove_from_cuts(node);
	
	jive_input_edge_iterator i = jive_node_iterate_inputs(node);
	while(i) {
		jive_edge * edge = i;
		i = jive_input_edge_iterator_next(i);
		jive_node * origin = edge->origin.node;
		jive_edge_remove(edge);
		check_bottomup_traversers(origin);
		_jive_node_prune_recursive(origin);
	}
	
	edge = head->input_edges.first;
	DEBUG_ASSERT(edge->base.origin.node == 0);
	DEBUG_ASSERT(edge->input_list.next == 0);
	REMOVE(&node->graph->null_origin, edge, output_list);
	
	edge = head->output_edges.first;
	DEBUG_ASSERT(edge->base.target.node == 0);
	DEBUG_ASSERT(edge->output_list.next == 0);
	REMOVE(&node->graph->null_target, edge, input_list);
	
	return true;
}

static void
recursive_recalculate_depth_from_root(jive_node * node)
{
	size_t old_depth = node->depth_from_root;
	/* reset to minimum -- node may have moved
	to top of graph */
	node->depth_from_root = 0;
	jive_input_edge_iterator i;
	JIVE_ITERATE_INPUTS(i, node) {
		size_t tmp = i->origin.node->depth_from_root + 1;
		if (tmp > node->depth_from_root)
			node->depth_from_root = tmp;
	}
	/* there is no need to revalidate lower nodes
	if depth has not changed */
	if (node->depth_from_root == old_depth) return;
	
	jive_output_edge_iterator o;
	JIVE_ITERATE_OUTPUTS(o, node)
		recursive_recalculate_depth_from_root(o->target.node);
}

/* public API starts here */

/* graph functions */

jive_graph *
jive_graph_create(jive_context * context)
{
	jive_graph * graph = jive_context_malloc(context, sizeof(*graph));
	
	graph->context = context;
	graph->top = graph->bottom = 0;
	jive_edgelist_init(&graph->null_origin);
	jive_edgelist_init(&graph->null_target);
	jive_edgelist_init(&graph->unused);
	
	graph->cached_traversers = 0;
	graph->first_traverser = graph->last_traverser = 0;
	graph->traversal_cookies[0] = graph->traversal_cookies[1] = 0;
	graph->allocated_traversers = 0;
	
	return graph;
}

/* FIXME: jive_graph_copy still missing */

void *
jive_malloc(jive_graph * graph, size_t count)
{
	return jive_context_malloc(graph->context, count);
}

const char *
jive_strdup(jive_graph * graph, const char * src)
{
	size_t namelen = strlen(src);
	char * dst = jive_malloc(graph, namelen+1);
	strcpy(dst, src);
	return dst;
}

void
jive_graph_fatal_error(jive_graph * graph, const char * message)
{
	return jive_context_fatal_error(graph->context, message);
}

jive_endian
jive_graph_endian(const jive_graph * graph)
{
	/* FIXME: return correct endian */
	return jive_endian_little;
}

void
jive_graph_prune(jive_graph * graph)
{
	jive_edge_container ** edge = &graph->null_target.first;
	
	while(*edge) {
		jive_node * node = (*edge)->base.origin.node;
		
		if (!_jive_node_prune_recursive(node))
			edge = &(*edge)->input_list.next;
	}
}

/* node functions */

void
jive_node_invalidate(jive_node * node)
{
	if (node->type->invalidate_inputs) node->type->invalidate_inputs(node);
}

void
jive_node_revalidate(jive_node * node)
{
	if (node->type->revalidate_outputs) node->type->revalidate_outputs(node);
}

void
jive_node_reserve(jive_node * node)
{
	head_of_node(node)->reservation++;
}

void
jive_node_unreserve(jive_node * node)
{
	head_of_node(node)->reservation--;
}

void
jive_node_prune_recursive(jive_node * node)
{
	_jive_node_prune_recursive(node);
}

bool
jive_node_is_instance(const jive_node * node, const jive_node_class * type)
{
	const jive_node_class * node_type = node->type;
	
	while(node_type) {
		if (node_type == node->type) return true;
		node_type = node_type->parent;
	}
	
	return false;
}

/* inputs */

void
jive_operand_init(jive_operand * input, jive_value * value)
{
	input->value = value;
	input->next = 0;
	input->extra = 0;
}

jive_operand *
jive_node_iterate_operands(jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	return head->inputs.first;
}

typedef struct _jive_orpend_base {
	JIVE_OPERAND_COMMON_FIELDS(jive_value)
	unsigned short index;
} jive_operand_base;

static char *
jive_operand_base_repr(const void * _self)
{
	const jive_operand_base * self = _self;
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "%d", self->index);
	return strdup(tmp);
}

const jive_operand_class JIVE_OPERAND_BASE = {
	0, "default numbered operand", sizeof(jive_operand_base),
	
	&jive_operand_base_repr
};

void jive_operand_base_init(jive_operand_base * input, jive_value * inout, unsigned int index)
{
	jive_operand_init((jive_operand *)input, inout);
	input->index = index;
}

jive_operand_list *
jive_operand_list_create(jive_graph *graph, size_t ninputs, jive_value * const values[])
{
	if (!ninputs) return 0;
	
	jive_operand_base * operand_list = jive_malloc(graph, sizeof(jive_operand_base) * ninputs);
	size_t n;
	for(n=0; n<ninputs; n++) {
		operand_list[n].type = &JIVE_OPERAND_BASE;
		jive_operand_base_init(&operand_list[n], values[n], n);
	}
	
	return (jive_operand_list *)operand_list;
}

/* auxiliary state */

jive_cpureg_class_t
jive_operand_get_cpureg_class(const jive_operand * input)
{
	return jive_input_get_extra_ro(input)->cpureg_cls;
}

void
jive_operand_set_cpureg_class(jive_operand * input, jive_cpureg_class_t regcls)
{
	jive_input_get_extra(input)->cpureg_cls = regcls;
}

jive_passthrough *
jive_operand_get_passthrough(const jive_operand * input)
{
	return jive_input_get_extra_ro(input)->passthrough;
}


/* FIXME: to be removed */
jive_node *
jive_node_alloc(jive_graph * graph, const jive_node_class * type, size_t ninputs, jive_value * const outputs[])
{
	jive_operand_list * input_list = jive_operand_list_create(graph, ninputs, outputs);
	
	return jive_node_create(graph, type, ninputs, input_list);
}

static inline void
jive_operand_value_connect_internal(
	jive_node * value_node,
	jive_value * value,
	jive_node * operand_node,
	jive_operand * operand)
{
	jive_edge_container * edge = jive_graph_edge_allocate(value_node->graph);
		
	edge->base.origin.node = value_node;
	edge->base.origin.port = value;
	edge->base.target.node = operand_node;
	edge->base.target.port = operand;
		
	PUSH_BACK(&head_of_node(operand_node)->input_edges, edge, input_list);
	PUSH_BACK(&head_of_node(value_node)->output_edges, edge, output_list);
}

void
jive_operand_value_connect(
	jive_node * value_node,
	jive_value * value,
	jive_node * operand_node,
	jive_operand * operand)
{
	mark_node_input_connected(operand_node->graph, operand_node);
	mark_node_output_connected(value_node->graph, value_node);
	jive_operand_value_connect_internal(
		value_node, value,
		operand_node, operand);
}

jive_node *
jive_node_create(
	jive_graph *graph,
	const jive_node_class * type,
	size_t ninputs,
	jive_operand_list * _inputs)
{
	void * inputs = _inputs;
	jive_node_head * head;
	jive_node * node;
	jive_graph_traverser * trav;
	size_t n;
	
	head = jive_context_malloc(graph->context, sizeof(*head)+type->size);
	jive_edgelist_init(&head->input_edges);
	jive_edgelist_init(&head->output_edges);
	head->inputs.first = head->inputs.last = 0;
	head->outputs.first = head->outputs.last = 0;
	head->gates = 0;
	head->traversal[0].cookie = head->traversal[1].cookie = 0;
	head->reservation = 0;
	
	node = (jive_node *)(head+1);
	node->type = type;
	node->graph = graph;
	node->depth_from_root = 0;
	
	for(n=0; n<ninputs; n++) {
		jive_operand * input = inputs;
		inputs = ((char *) inputs) + input->type->size;
		
		jive_node_add_operand(node, input);
		
		jive_value * origin_port = input->value;
		jive_node * origin_node = origin_port->node;
		if (origin_node->depth_from_root+1 > node->depth_from_root)
			node->depth_from_root = origin_node->depth_from_root+1;
		
		mark_node_output_connected(origin_node->graph, origin_node);
		
		jive_operand_value_connect_internal(
			origin_node, origin_port,
			node, input);
	}
	
	if (!ninputs)
		mark_dangling_node_input(graph, node);
	
	JIVE_ITERATE_ACTIVE_TRAVERSERS(graph, trav) {
		/* tentatively mark this node as "behind" by all concurrent traversers */
		mark_node_behind_cut(trav, node);
	
		if (trav->direction != +1) continue;
		/* special treatment for top-down traversers: newly created nodes
		will subsequently be visited iff input nodes have not been visited yet;
		make sure cookie has correct value by picking "minimum" of all input
		nodes */
		if (!all_inputs_behind_cut(trav, node))
			mark_node_before_cut(trav, node);
	}
	
	mark_dangling_node_output(graph, node);
	
	return node;
}

void
jive_node_add_value(jive_node * node, jive_value * value)
{
	jive_node_head * head = head_of_node(node);
	
	if (head->outputs.first == value) abort();
	
	value->next = 0;
	if (head->outputs.last) head->outputs.last->next = value;
	else head->outputs.first = value;
	head->outputs.last = value;
}

void
jive_node_add_operand(jive_node * node, jive_operand * input)
{
	jive_node_head * head = head_of_node(node);
	input->next = 0;
	if (head->inputs.last) head->inputs.last->next = input;
	else head->inputs.first = input;
	head->inputs.last = input;
}

jive_edge *
jive_state_edge_create(jive_node * origin, jive_node * target)
{
	jive_edge_container * edge;
	
	mark_node_input_connected(target->graph, target);
	mark_node_output_connected(origin->graph, origin);
	
	edge = jive_graph_edge_allocate(origin->graph);
	
	edge->base.origin.node = origin;
	edge->base.origin.port = 0;
	edge->base.target.node = target;
	edge->base.target.port = 0;
	
	PUSH_FRONT(&head_of_node(target)->input_edges, edge, input_list);
	PUSH_FRONT(&head_of_node(origin)->output_edges, edge, output_list);
	
	check_topdown_traversers(target);
	check_bottomup_traversers(origin);
	
	return &edge->base;
}

void
jive_state_edge_remove(jive_edge * edge)
{
	jive_node * node = edge->target.node;
	jive_edge_remove(edge);
	recursive_recalculate_depth_from_root(node);
}

bool
jive_edge_is_state_edge(const jive_edge * edge)
{
	return edge->origin.port == 0;
}

void
jive_edge_divert_origin(jive_edge * _edge, jive_value * origin_port)
{
	jive_edge_container * edge = (jive_edge_container *)_edge;
	jive_node * old_origin = edge->base.origin.node;
	jive_node * new_origin = origin_port->node;
	REMOVE(&head_of_node(old_origin)->output_edges, edge, output_list);
	PUSH_BACK(&head_of_node(new_origin)->output_edges, edge, output_list);
	edge->base.origin.node = new_origin;
	edge->base.origin.port = origin_port;
	
	edge->base.target.port->value = origin_port;
	
	if (!head_of_node(old_origin)->output_edges.first) mark_dangling_node_output(old_origin->graph, old_origin);
	
	check_bottomup_traversers(old_origin);
	check_bottomup_traversers(new_origin);
	check_topdown_traversers(edge->base.target.node);
	recursive_recalculate_depth_from_root(_edge->target.node);
}

void
jive_state_edge_divert_origin(jive_edge * _edge, jive_node * new_origin)
{
	jive_edge_container * edge = (jive_edge_container *)_edge;
	jive_node * old_origin = edge->base.origin.node;
	REMOVE(&head_of_node(old_origin)->output_edges, edge, output_list);
	PUSH_FRONT(&head_of_node(new_origin)->output_edges, edge, output_list);
	edge->base.origin.node=new_origin;
	edge->base.origin.port=0;
	
	if (!head_of_node(old_origin)->output_edges.first) mark_dangling_node_output(old_origin->graph, old_origin);
	
	check_bottomup_traversers(old_origin);
	check_bottomup_traversers(new_origin);
	check_topdown_traversers(edge->base.target.node);
	recursive_recalculate_depth_from_root(_edge->target.node);
}

void
jive_value_init(jive_value * output, jive_node * node)
{
	output->node = node;
	output->next = 0;
	output->extra = 0;
	jive_node_add_value(node, output);
}

jive_value *
jive_node_iterate_values(jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	return head->outputs.first;
}

jive_cpureg_class_t
jive_value_get_cpureg_class(const jive_value * value)
{
	return jive_value_get_extra_ro(value)->cpureg_cls;
}

void
jive_value_set_cpureg_class(jive_value * value, jive_cpureg_class_t regcls)
{
	jive_value_get_extra(value)->cpureg_cls = regcls;
}

jive_cpureg_class_t
jive_value_get_cpureg_class_shared(const jive_value * value)
{
	return jive_value_get_extra_ro(value)->cpureg_cls_shared;
}

void
jive_value_set_cpureg_class_shared(jive_value * value, jive_cpureg_class_t regcls)
{
	jive_value_get_extra(value)->cpureg_cls_shared = regcls;
}

jive_cpureg_t
jive_value_get_cpureg(const jive_value * value)
{
	return jive_value_get_extra_ro(value)->cpureg;
}

void
jive_value_set_cpureg(jive_value * value, jive_cpureg_t reg)
{
	jive_value_get_extra(value)->cpureg = reg;
}

jive_passthrough *
jive_value_get_passthrough(const jive_value * value)
{
	return jive_value_get_extra_ro(value)->passthrough;
}

jive_regalloc_regstate
jive_value_get_regalloc_regstate(const jive_value * value)
{
	return jive_value_get_extra_ro(value)->ra_state;
}

void
jive_value_set_regalloc_regstate(jive_value * value, jive_regalloc_regstate state)
{
	jive_value_get_extra(value)->ra_state = state;
}

bool
jive_value_get_mayspill(const jive_value * value)
{
	return jive_value_get_extra_ro(value)->may_spill;
}

void
jive_value_set_mayspill(jive_value * value, bool may_spill)
{
	jive_value_get_extra(value)->may_spill = may_spill;
}


void
jive_value_replace(jive_value * old_port, jive_value * new_port)
{
	jive_node * old_origin = old_port->node;
	jive_node * new_origin = new_port->node;
	
	jive_output_edge_iterator o = jive_node_iterate_outputs(old_origin);
	while(o) {
		jive_edge_container * edge = (jive_edge_container *)o;
		o = jive_output_edge_iterator_next(o);
		
		if (edge->base.origin.port != old_port) continue;
		
		REMOVE(&head_of_node(old_origin)->output_edges, edge, output_list);
		PUSH_BACK(&head_of_node(new_origin)->output_edges, edge, output_list);
		
		edge->base.origin.node = new_origin;
		edge->base.origin.port = new_port;
		edge->base.target.port->value = new_port;
		
		check_topdown_traversers(edge->base.target.node);
		recursive_recalculate_depth_from_root(edge->base.target.node);
	}
	
	if (!head_of_node(old_origin)->output_edges.first)
		mark_dangling_node_output(old_origin->graph, old_origin);
	
	check_bottomup_traversers(old_origin);
	check_bottomup_traversers(new_origin);
}

bool
jive_value_is_instance(const jive_value * value, const jive_value_class * type)
{
	const jive_value_class * cls = value->type;
	while(cls) {
		if (cls == type) return true;
		cls = cls->parent;
	}
	return false;
}

/* edge iterators */

jive_input_edge_iterator
jive_node_iterate_inputs(jive_node * node)
{
	jive_edge_container * edge = head_of_node(node)->input_edges.first;
	if (!edge->base.origin.node) edge = edge->input_list.next;
	return &edge->base;
}

jive_input_edge_iterator
jive_input_edge_iterator_next(jive_input_edge_iterator iterator)
{
	return &((jive_edge_container *)iterator)->input_list.next->base;
}

jive_output_edge_iterator
jive_node_iterate_outputs(jive_node * node)
{
	jive_edge_container * edge = head_of_node(node)->output_edges.first;
	if (!edge->base.target.node) edge = edge->output_list.next;
	return &edge->base;
}

jive_output_edge_iterator
jive_output_edge_iterator_next(jive_output_edge_iterator iterator)
{
	return &((jive_edge_container *)iterator)->output_list.next->base;
}

jive_output_edge_iterator
jive_graph_iterate_top(jive_graph * graph)
{
	return &graph->null_origin.first->base;
}

jive_input_edge_iterator
jive_graph_iterate_bottom(jive_graph * graph)
{
	return &graph->null_target.first->base;
}

/* traversal */

static jive_graph_traverser *
jive_graph_traverser_get(jive_graph * graph)
{
	jive_graph_traverser * traverser;
	if (graph->cached_traversers) {
		traverser = graph->cached_traversers;
		graph->cached_traversers = traverser->next;
		traverser->next = 0;
	} else {
		traverser = jive_context_malloc(graph->context, sizeof(*traverser));
		traverser->graph = graph;
		traverser->index = graph->allocated_traversers++;
		
		if (graph->allocated_traversers > CONCURRENT_TRAVERSALS)
			jive_context_fatal_error(graph->context, "Internal error: Too many concurrent traversers");
	}
	
	traverser->cut.first = traverser->cut.last = 0;
	
	graph->traversal_cookies[traverser->index] += 2;
	traverser->cookie = graph->traversal_cookies[traverser->index];
	
	/* link into list of active traversers */
	if (graph->last_traverser) graph->last_traverser->next = traverser;
	else graph->first_traverser = traverser;
	traverser->prev = graph->last_traverser;
	traverser->next = 0;
	graph->last_traverser = traverser;
	
	return traverser;
}

static void
jive_graph_traverser_put(jive_graph_traverser * traverser)
{
	jive_graph * graph = traverser->graph;
	
	/* unlink from list of active traversers */
	if (traverser->prev) traverser->prev->next = traverser->next;
	else graph->first_traverser = traverser->next;
	if (traverser->next) traverser->next->prev = traverser->prev;
	else graph->last_traverser = traverser->prev;
	
	traverser->next = traverser->graph->cached_traversers;
	graph->cached_traversers = traverser;
}

jive_graph_traverser *
jive_graph_traverse_topdown(jive_graph * graph)
{
	jive_graph_traverser * traverser = jive_graph_traverser_get(graph);
	
	jive_edge_container * edge = graph->null_origin.first;
	
	while(edge) {
		add_to_cut(traverser, edge->base.target.node);
		edge = edge->output_list.next;
	}
	
	traverser->direction = +1;
	
	return traverser;
}

jive_graph_traverser *
jive_graph_traverse_bottomup(jive_graph * graph)
{
	jive_graph_traverser * traverser = jive_graph_traverser_get(graph);
	
	jive_edge_container * edge = graph->null_target.first;
	
	while(edge) {
		add_to_cut(traverser, edge->base.origin.node);
		edge = edge->input_list.next;
	}
	
	traverser->direction = -1;
	
	return traverser;
}

void
jive_graph_traverse_finish(jive_graph_traverser * traverser)
{
	while(traverser->cut.first)
		remove_from_cut(traverser, traverser->cut.first);
	jive_graph_traverser_put(traverser);
}

jive_node *
jive_graph_traverse_next(jive_graph_traverser * traverser)
{
	jive_node * node = traverser->cut.first;
	if (!node) return 0;
	
	remove_from_cut(traverser, node);
	DEBUG_ASSERT( traversal_state(traverser, node)->cookie < traverser->cookie );
	DEBUG_ASSERT( traversal_state(traverser, node)->cookie & 1 );
	traversal_state(traverser, node)->cookie = traverser->cookie;
	
	jive_input_edge_iterator i;
	jive_output_edge_iterator o;
	
	if (traverser->direction == +1) {
		JIVE_ITERATE_OUTPUTS(o, node) {
			jive_node * tmp = o->target.node;
			JIVE_ITERATE_INPUTS(i, tmp) {
				if (traversal_state(traverser, i->origin.node)->cookie != traverser->cookie)
					break;
			}
			if (i) continue;
			DEBUG_ASSERT( traversal_state(traverser, tmp)->cookie < traverser->cookie );
			add_to_cut(traverser, tmp);
		}
	} else {
		JIVE_ITERATE_INPUTS(i, node) {
			jive_node * tmp = i->origin.node;
			JIVE_ITERATE_OUTPUTS(o, tmp) {
				if (traversal_state(traverser, o->target.node)->cookie != traverser->cookie)
					break;
			}
			if (o) continue;
			DEBUG_ASSERT( traversal_state(traverser, tmp)->cookie < traverser->cookie );
			add_to_cut(traverser, tmp);
		}
	}
	
	return node;
}

