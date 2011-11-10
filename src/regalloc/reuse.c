#include <jive/regalloc/reuse.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>

#include <jive/regalloc/shaped-graph.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/variable.h>

typedef struct jive_reuse_type jive_reuse_type;
typedef struct jive_reuse_input jive_reuse_input;
typedef struct jive_reuse_output jive_reuse_output;
typedef struct jive_reuse_output jive_reuse;
typedef struct jive_reuse_gate jive_reuse_gate;
typedef struct jive_reuse_resource jive_reuse_resource;

extern const jive_type_class JIVE_REUSE_TYPE;
#define JIVE_DECLARE_REUSE_TYPE(name_, resname) const jive_reuse_type name_##_struct = {{{&JIVE_REUSE_TYPE}}, resname}; const jive_type * name_ = &name_##_struct.base.base

struct jive_reuse_type {
	jive_state_type base;
	const jive_resource_name * name;
};

extern const jive_input_class JIVE_REUSE_INPUT;
struct jive_reuse_input {
	jive_state_input base;
	jive_reuse_type type;
};

extern const jive_output_class JIVE_REUSE_OUTPUT;
struct jive_reuse_output {
	jive_state_output base;
	jive_reuse_type type;
};

extern const jive_gate_class JIVE_REUSE_GATE;
struct jive_reuse_gate {
	jive_state_gate base;
	jive_reuse_type type;
};

static void
jive_reuse_type_init_(jive_reuse_type * self, const jive_resource_name * name)
{
	self->base.base.class_ = &JIVE_REUSE_TYPE;
	self->name = name;
}


static void
jive_reuse_input_init_(jive_reuse_input * self, struct jive_node * node, size_t index, jive_output * origin, const jive_resource_name * name)
{
	_jive_state_input_init(&self->base, node, index, origin);
	jive_reuse_type_init_(&self->type, name);
}

static const jive_type *
jive_reuse_input_get_type_(const jive_input * self_)
{
	const jive_reuse_input * self = (const jive_reuse_input *) self_;
	return &self->type.base.base;
}

static void
jive_reuse_output_init_(jive_reuse_output * self, struct jive_node * node, size_t index, const jive_resource_name * name)
{
	_jive_state_output_init(&self->base, node, index);
	jive_reuse_type_init_(&self->type, name);
}

static const jive_type *
jive_reuse_output_get_type_(const jive_output * self_)
{
	const jive_reuse_output * self = (const jive_reuse_output *) self_;
	return &self->type.base.base;
}

void
jive_reuse_gate_init_(jive_reuse_gate * self, struct jive_graph * graph, const char * name, const jive_resource_name * resname)
{
	_jive_state_gate_init(&self->base, graph, name);
	jive_reuse_type_init_(&self->type, resname);
}

const jive_type *
jive_reuse_gate_get_type_(const jive_gate * self_)
{
	const jive_reuse_gate * self = (const jive_reuse_gate *) self_;
	return &self->type.base.base;
}

static jive_type *
jive_reuse_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	
	jive_reuse_type * type = jive_context_malloc(context, sizeof(*type));
	
	jive_reuse_type_init_(type, self->name);
	
	return &type->base.base;
}

static char *
jive_reuse_type_get_label_(const jive_type * self_)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", self->name->name);
	return strdup(tmp);
}

static jive_input *
jive_reuse_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	
	jive_reuse_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_REUSE_INPUT;
	jive_reuse_input_init_(input, node, index, initial_operand, self->name);
	return &input->base.base;
}

static jive_output *
jive_reuse_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	
	jive_reuse_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_REUSE_OUTPUT;
	jive_reuse_output_init_(output, node, index, self->name);
	return &output->base.base;
}

static jive_gate *
jive_reuse_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	
	jive_reuse_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_REUSE_GATE;
	jive_reuse_gate_init_(gate, graph, name, self->name);
	
	return &gate->base.base;
}

const jive_type_class JIVE_REUSE_TYPE = {
	.parent = &JIVE_TYPE,
	.fini = _jive_state_type_fini, /* inherit */
	.copy = jive_reuse_type_copy_, /* override */
	.get_label = jive_reuse_type_get_label_, /* inherit */
	.create_input = jive_reuse_type_create_input_, /* override */
	.create_output = jive_reuse_type_create_output_, /* override */
	.create_gate = jive_reuse_type_create_gate_, /* override */
	.equals = jive_type_equals_, /* inherit */
};

const jive_input_class JIVE_REUSE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = jive_input_fini_, /* inherit */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_reuse_input_get_type_, /* override */
};

const jive_output_class JIVE_REUSE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_reuse_output_get_type_, /* override */
};

const jive_gate_class JIVE_REUSE_GATE = {
	.parent = &JIVE_GATE,
	.fini = jive_gate_fini_, /* inherit */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = jive_reuse_gate_get_type_, /* override */
};

/* structures for tracking current active set and users */

typedef struct jive_node_vector jive_node_vector;
struct jive_node_vector {
	size_t nitems, space;
	jive_node ** items;
};

static inline void
jive_node_vector_init(jive_node_vector * self)
{
	self->nitems = self->space = 0;
	self->items = 0;
}

static inline void
jive_node_vector_fini(jive_node_vector * self, jive_context * context)
{
	jive_context_free(context, self->items);
}

static inline void
jive_node_vector_clear(jive_node_vector * self)
{
	self->nitems = 0;
}

static inline void
jive_node_vector_push_back(jive_node_vector * self, jive_node * node, jive_context * context)
{
	if (self->nitems == self->space) {
		self->space = self->space * 2 + 1;
		self->items = jive_context_realloc(context, self->items, sizeof(self->items[0]) * self->space);
	}
	self->items[self->nitems ++] = node;
}

typedef struct jive_used_name jive_used_name;
struct jive_used_name {
	const jive_resource_name * name;
	jive_node_vector read;
	jive_node_vector clobber;
	size_t read_count, write_count;
	struct {
		jive_used_name * prev;
		jive_used_name * next;
	} hash_chain;
	struct {
		jive_used_name * prev;
		jive_used_name * next;
	} used_names_list;
};

JIVE_DECLARE_HASH_TYPE(jive_used_name_hash, jive_used_name, const jive_resource_name *, name, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_used_name_hash, jive_used_name, const jive_resource_name *, name, hash_chain);
typedef struct jive_used_name_hash jive_used_name_hash;

typedef struct jive_names_use jive_names_use;
struct jive_names_use {
	jive_used_name_hash hash;
	struct {
		jive_used_name * first;
		jive_used_name * last;
	} list;
};

static void
jive_names_use_init(jive_names_use * self, jive_context * context)
{
	jive_used_name_hash_init(&self->hash, context);
	self->list.first = self->list.last = 0;
}

static jive_used_name *
jive_names_use_lookup(jive_names_use * self, const jive_resource_name * name)
{
	JIVE_DEBUG_ASSERT(name);
	jive_used_name * used_name = jive_used_name_hash_lookup(&self->hash, name);
	if (!used_name) {
		used_name = jive_context_malloc(self->hash.context, sizeof(*used_name));
		used_name->name = name;
		jive_node_vector_init(&used_name->read);
		jive_node_vector_init(&used_name->clobber);
		used_name->read_count = 0;
		used_name->write_count = 0;
		JIVE_LIST_PUSH_BACK(self->list, used_name, used_names_list);
		jive_used_name_hash_insert(&self->hash, used_name);
	}
	
	return used_name;
}

static void
jive_names_use_remove(jive_names_use * self, jive_used_name * used_name)
{
	jive_context * context = self->hash.context;
	jive_node_vector_fini(&used_name->read, context);
	jive_node_vector_fini(&used_name->clobber, context);
	jive_used_name_hash_remove(&self->hash, used_name);
	JIVE_LIST_REMOVE(self->list, used_name, used_names_list);
	jive_context_free(context, used_name);
}

static void
jive_names_use_fini(jive_names_use * self)
{
	while (self->list.first) {
		jive_used_name * used_name = self->list.first;
		jive_names_use_remove(self, used_name);
	}
	jive_used_name_hash_fini(&self->hash);
}

static void
jive_names_use_read(jive_names_use * self, const jive_resource_name * name, jive_node * node)
{
	jive_used_name * used_name = jive_names_use_lookup(self, name);
	
	jive_node_vector_push_back(&used_name->read, node, self->hash.context);
	used_name->read_count ++;
}

static void
jive_names_use_clobber(jive_names_use * self, const jive_resource_name * name, jive_node * node)
{
	jive_used_name * used_name = jive_names_use_lookup(self, name);
	
	JIVE_DECLARE_REUSE_TYPE(type, name);
	size_t n;
	for (n = 0; n < used_name->read.nitems; n++) {
		jive_node * before = used_name->read.items[n];
		
		if (node != before)
			jive_node_add_input(node, type, jive_node_add_output(before, type));
	}
	
	jive_node_vector_push_back(&used_name->clobber, node, self->hash.context);
	used_name->write_count ++;
}

static void
jive_names_use_write(jive_names_use * self, const jive_resource_name * name, jive_node * node)
{
	jive_used_name * used_name = jive_names_use_lookup(self, name);
	
	JIVE_DECLARE_REUSE_TYPE(type, name);
	
	size_t n;
	for (n = 0; n < used_name->read.nitems; n++) {
		jive_node * before = used_name->read.items[n];
		if (node != before)
			jive_node_add_input(node, type, jive_node_add_output(before, type));
	}
	for (n = 0; n < used_name->clobber.nitems; n++) {
		jive_node * before = used_name->clobber.items[n];
		if (node != before)
			jive_node_add_input(node, type, jive_node_add_output(before, type));
	}
	
	jive_node_vector_clear(&used_name->read);
	jive_node_vector_clear(&used_name->clobber);
	used_name->write_count ++;
}

/* add reuse edges to graph */

static void
jive_regalloc_reuse_record_region(jive_shaped_graph * shaped_graph, jive_region * region, jive_names_use * names_use);

static void
jive_regalloc_reuse_record_node(jive_shaped_graph * shaped_graph, jive_node * node, jive_names_use * names_use)
{
	jive_context * context = shaped_graph->context;
	
	jive_names_use sub_names_use;
	jive_names_use_init(&sub_names_use, context);
	jive_used_name * used_name;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (!jive_input_isinstance(input, &JIVE_ANCHOR_INPUT))
			continue;
		
		jive_names_use inner_names_use;
		jive_names_use_init(&inner_names_use, context);
		
		jive_regalloc_reuse_record_region(shaped_graph, input->origin->node->region, &inner_names_use);
		
		jive_used_name * inner_used_name;
		JIVE_LIST_ITERATE(inner_names_use.list, inner_used_name, used_names_list) {
			jive_used_name * used_name = jive_names_use_lookup(&sub_names_use, inner_used_name->name);
			used_name->read_count += inner_used_name->read_count;
			used_name->write_count += inner_used_name->write_count;
		}
		
		jive_names_use_fini(&inner_names_use);
	}
	
	JIVE_LIST_ITERATE(sub_names_use.list, used_name, used_names_list) {
		if (used_name->read_count)
			jive_names_use_read(names_use, used_name->name, node);
	}
	
	for (n = 0; n < node->ninputs; n++) {
		jive_ssavar * ssavar = node->inputs[n]->ssavar;
		if (!ssavar)
			continue;
		const jive_resource_name * resname = jive_variable_get_resource_name(ssavar->variable);
		if (!resname)
			continue;
		jive_names_use_read(names_use, resname, node);
	}
	
	JIVE_LIST_ITERATE(sub_names_use.list, used_name, used_names_list) {
		if (used_name->write_count)
			jive_names_use_clobber(names_use, used_name->name, node);
	}
	
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_ssavar * ssavar = output->ssavar;
		if (!ssavar)
			continue;
		const jive_resource_name * resname = jive_variable_get_resource_name(ssavar->variable);
		if (!resname)
			continue;
		
		if (output->users.first)
			jive_names_use_write(names_use, resname, node);
		else
			jive_names_use_clobber(names_use, resname, node);
	}
	
	jive_names_use_fini(&sub_names_use);
}

static void
jive_regalloc_reuse_record_region(jive_shaped_graph * shaped_graph, jive_region * region, jive_names_use * names_use)
{
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, region);
	
	jive_cut * cut;
	JIVE_LIST_ITERATE(shaped_region->cuts, cut, region_cut_list) {
		jive_shaped_node * shaped_node;
		JIVE_LIST_ITERATE(cut->locations, shaped_node, cut_location_list) {
			jive_node * node = shaped_node->node;
			jive_regalloc_reuse_record_node(shaped_graph, node, names_use);
		}
	}
}

void
jive_regalloc_reuse(jive_shaped_graph * shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	jive_context * context = graph->context;
	
	jive_names_use names_use;
	jive_names_use_init(&names_use, context);
	
	jive_regalloc_reuse_record_region(shaped_graph, graph->root_region, &names_use);
	
	jive_names_use_fini(&names_use);
}
