#include <jive/regalloc/regreuse.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/cut.h>
#include <jive/arch/registers.h>
#include <jive/context.h>
#include <jive/debug-private.h>
#include <stdio.h>
#include <string.h>

typedef struct jive_regreuse_type jive_regreuse_type;
typedef struct jive_regreuse_input jive_regreuse_input;
typedef struct jive_regreuse_output jive_regreuse_output;
typedef struct jive_regreuse_gate jive_regreuse_gate;
typedef struct jive_regreuse_resource jive_regreuse_resource;

struct jive_regreuse_type {
	jive_state_type base;
	const jive_cpureg * reused_reg;
};

struct jive_regreuse_input {
	jive_state_input base;
	jive_regreuse_type type;
};

struct jive_regreuse_output {
	jive_state_output base;
	jive_regreuse_type type;
};

struct jive_regreuse_resource {
	jive_state_resource base;
	jive_regreuse_type type;
};

static jive_input *
_jive_regreuse_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);

static jive_output *
_jive_regreuse_type_create_output(const jive_type * self, struct jive_node * node, size_t index);

static jive_resource *
_jive_regreuse_type_create_resource(const jive_type * self, struct jive_graph * graph);

static jive_gate *
_jive_regreuse_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name);

static char *
_jive_regreuse_type_get_label(const jive_type * self);

static void
_jive_regreuse_input_init(jive_regreuse_input * self, const jive_cpureg * reused_reg, struct jive_node * node, size_t index, jive_output * origin);

static const jive_type *
_jive_regreuse_input_get_type(const jive_input * self);

static char *
_jive_regreuse_input_get_label(const jive_input * self);

static void
_jive_regreuse_output_init(jive_regreuse_output * self, const jive_cpureg * reused_reg, struct jive_node * node, size_t index);

static const jive_type *
_jive_regreuse_output_get_type(const jive_output * self);

static char *
_jive_regreuse_output_get_label(const jive_output * self);

static void
_jive_regreuse_resource_init(jive_regreuse_resource * self, const jive_cpureg * reused_reg, struct jive_graph * graph);

static const jive_type *
_jive_regreuse_resource_get_type(const jive_resource * self);

static char *
_jive_regreuse_resource_get_label(const jive_resource * self_);

static const jive_type_class JIVE_REGREUSE_TYPE = {
	.parent = &JIVE_TYPE,
	.get_label = _jive_regreuse_type_get_label, /* override */
	.create_input = _jive_regreuse_type_create_input, /* override */
	.create_output = _jive_regreuse_type_create_output, /* override */
	.create_resource = _jive_regreuse_type_create_resource, /* override */
	.create_gate = _jive_regreuse_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
	.accepts = _jive_type_accepts /* inherit */
};

static const jive_input_class JIVE_REGREUSE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_regreuse_input_get_label, /* override */
	.get_type = _jive_regreuse_input_get_type, /* override */
	.get_constraint = _jive_input_get_constraint /* inherit */
};

static const jive_output_class JIVE_REGREUSE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_regreuse_output_get_label, /* override */
	.get_type = _jive_regreuse_output_get_type, /* override */
	.get_constraint = _jive_output_get_constraint /* inherit */
};

static const jive_resource_class JIVE_REGREUSE_RESOURCE = {
	.parent = &JIVE_RESOURCE,
	.fini = _jive_resource_fini, /* inherit */
	.get_label = _jive_regreuse_resource_get_label, /* override */
	.get_type = _jive_regreuse_resource_get_type, /* override */
	.can_merge = _jive_resource_can_merge, /* inherit */
	.merge = _jive_resource_merge, /* inherit */
	.get_cpureg = _jive_resource_get_cpureg, /* inherit */
	.get_regcls = _jive_resource_get_regcls, /* inherit */
	.get_real_regcls = _jive_resource_get_real_regcls, /* inherit */
	.add_squeeze = _jive_resource_add_squeeze, /* inherit */
	.sub_squeeze = _jive_resource_sub_squeeze, /* inherit */
	.deny_register = _jive_resource_deny_register, /* inherit */
	.recompute_allowed_registers = _jive_resource_recompute_allowed_registers /* inherit */
};

static void
jive_regreuse_type_init(jive_regreuse_type * self, const jive_cpureg * reused_reg)
{
	self->base.base.class_ = &JIVE_REGREUSE_TYPE;
	self->reused_reg = reused_reg;
}

char *
_jive_regreuse_type_get_label(const jive_type * self_)
{
	const jive_regreuse_type * self = (const jive_regreuse_type *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", self->reused_reg->name);
	return strdup(tmp);
}

jive_input *
_jive_regreuse_type_create_input(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_regreuse_type * self = (const jive_regreuse_type *) self_;
	jive_regreuse_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_REGREUSE_INPUT;
	_jive_regreuse_input_init(input, self->reused_reg, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
_jive_regreuse_type_create_output(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_regreuse_type * self = (const jive_regreuse_type *) self_;
	jive_regreuse_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_REGREUSE_OUTPUT;
	_jive_regreuse_output_init(output, self->reused_reg, node, index);
	return &output->base.base;
}

jive_resource *
_jive_regreuse_type_create_resource(const jive_type * self_, struct jive_graph * graph)
{
	const jive_regreuse_type * self = (const jive_regreuse_type *) self_;
	jive_regreuse_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.base.class_ = &JIVE_REGREUSE_RESOURCE;
	_jive_regreuse_resource_init(resource, self->reused_reg, graph);
	return &resource->base.base;
}

jive_gate *
_jive_regreuse_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	return 0;
}

void
_jive_regreuse_input_init(jive_regreuse_input * self, const jive_cpureg * reused_reg, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_state_input_init(&self->base, node, index, origin);
	jive_regreuse_type_init(&self->type, reused_reg);
}

const jive_type *
_jive_regreuse_input_get_type(const jive_input * self_)
{
	jive_regreuse_input * self = (jive_regreuse_input *) self_;
	return &self->type.base.base;
}

char *
_jive_regreuse_input_get_label(const jive_input * self_)
{
	const jive_regreuse_input * self = (const jive_regreuse_input *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", self->type.reused_reg->name);
	return strdup(tmp);
}

void
_jive_regreuse_output_init(jive_regreuse_output * self, const jive_cpureg * reused_reg, struct jive_node * node, size_t index)
{
	_jive_state_output_init(&self->base, node, index);
	jive_regreuse_type_init(&self->type, reused_reg);
}

const jive_type *
_jive_regreuse_output_get_type(const jive_output * self_)
{
	jive_regreuse_output * self = (jive_regreuse_output *) self_;
	return &self->type.base.base;
}

char *
_jive_regreuse_output_get_label(const jive_output * self_)
{
	const jive_regreuse_output * self = (const jive_regreuse_output *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", self->type.reused_reg->name);
	return strdup(tmp);
}

void
_jive_regreuse_resource_init(jive_regreuse_resource * self, const jive_cpureg * reused_reg, struct jive_graph * graph)
{
	_jive_state_resource_init(&self->base, graph);
	jive_regreuse_type_init(&self->type, reused_reg);
}

const jive_type *
_jive_regreuse_resource_get_type(const jive_resource * self_)
{
	jive_regreuse_resource * self = (jive_regreuse_resource *) self_;
	return &self->type.base.base;
}

char *
_jive_regreuse_resource_get_label(const jive_resource * self_)
{
	const jive_regreuse_resource * self = (const jive_regreuse_resource *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", self->type.reused_reg->name);
	return strdup(tmp);
}

typedef struct jive_reguse jive_reguse;
struct jive_reguse {
	const jive_cpureg * reg;
	struct {
		size_t nitems, space;
		jive_node ** nodes;
	} read;
	struct {
		size_t nitems, space;
		jive_node ** nodes;
	} clobber;
	struct {
		jive_reguse * prev;
		jive_reguse * next;
	} hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_reguse_hash, jive_reguse, const jive_cpureg *, reg, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_reguse_hash, jive_reguse, const jive_cpureg *, reg, hash_chain);
typedef struct jive_reguse_hash jive_reguse_hash;

static jive_reguse *
jive_reguse_get(jive_reguse_hash * hash, const jive_cpureg * reg)
{
	jive_reguse * use = jive_reguse_hash_lookup(hash, reg);
	if (!use) {
		use = jive_context_malloc(hash->context, sizeof(*use));
		use->reg = reg;
		use->read.nitems = use->read.space = 0;
		use->read.nodes = 0;
		use->clobber.nitems = use->clobber.space = 0;
		use->clobber.nodes = 0;
		jive_reguse_hash_insert(hash, use);
	}
	return use;
}

static void
jive_reguse_destroy(jive_reguse * self, jive_reguse_hash * hash)
{
	jive_reguse_hash_remove(hash, self);
	jive_context_free(hash->context, self->read.nodes);
	jive_context_free(hash->context, self->clobber.nodes);
	jive_context_free(hash->context, self);
}

static void
jive_reguse_add_reader(jive_reguse * self, jive_node * node, jive_context * context)
{
	DEBUG_ASSERT(node);
	if (self->read.nitems >= self->read.space) {
		self->read.space = self->read.nitems * 2 + 1;
		self->read.nodes = jive_context_realloc(context, self->read.nodes, self->read.space * sizeof(jive_node *));
	}
	self->read.nodes[self->read.nitems ++] = node;
}

static void
jive_reguse_add_clobber(jive_reguse * self, jive_node * node, jive_context * context)
{
	DEBUG_ASSERT(node);
	if (self->clobber.nitems >= self->clobber.space) {
		self->clobber.space = self->clobber.nitems * 2 + 1;
		self->clobber.nodes = jive_context_realloc(context, self->clobber.nodes, self->clobber.space * sizeof(jive_node *));
	}
	self->clobber.nodes[self->clobber.nitems ++] = node;
}

static void
jive_reguse_clear(jive_reguse * self)
{
	self->read.nitems = 0;
	self->clobber.nitems = 0;
}

static void
add_reuse_edge(jive_node * from, jive_node * to, const jive_cpureg * cpureg)
{
	if (from == to) return;
	
	jive_regreuse_type type;
	jive_regreuse_type_init(&type, cpureg);
	
	jive_output * output = jive_node_add_output(from, &type.base.base);
	jive_node_add_input(to, &type.base.base, output);
}

static void
process_node(jive_node * node, jive_reguse_hash * hash)
{
	size_t n, k;
	for(n=0; n<node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		const jive_cpureg * cpureg = jive_resource_get_cpureg(input->resource);
		if (!cpureg) continue;
		jive_reguse * use = jive_reguse_get(hash, cpureg);
		jive_reguse_add_reader(use, node, hash->context);
	}
	
	for(n=0; n<node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		const jive_cpureg * cpureg = jive_resource_get_cpureg(output->resource);
		if (!cpureg) continue;
		jive_reguse * use = jive_reguse_get(hash, cpureg);
		if (output->users.first) {
			/* written value is used */
			for(k=0; k<use->read.nitems; k++) add_reuse_edge(use->read.nodes[k], node, cpureg);
			for(k=0; k<use->clobber.nitems; k++) add_reuse_edge(use->clobber.nodes[k], node, cpureg);
			jive_reguse_clear(use);
		} else {
			for(k=0; k<use->read.nitems; k++) add_reuse_edge(use->read.nodes[k], node, cpureg);
			jive_reguse_add_clobber(use, node, hash->context);
		}
	}
}

static void
process_region(jive_region * region)
{
	jive_reguse_hash hash;
	jive_reguse_hash_init(&hash, region->graph->context);
	
	jive_cut * cut;
	JIVE_LIST_ITERATE(region->cuts, cut, region_cuts_list) {
		jive_node_location * loc;
		JIVE_LIST_ITERATE(cut->nodes, loc, cut_nodes_list) {
			process_node(loc->node, &hash);
		}
	}
	
	struct jive_reguse_hash_iterator i = jive_reguse_hash_begin(&hash);
	while(i.entry) {
		jive_reguse * use = i.entry;
		jive_reguse_hash_iterator_next(&i);
		jive_reguse_destroy(use, &hash);
	}
	jive_reguse_hash_fini(&hash);
	
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		process_region(subregion);
}

void
jive_regalloc_regreuse(struct jive_graph * graph)
{
	process_region(graph->root_region);
}
