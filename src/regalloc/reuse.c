/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/reuse.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>

#include <jive/regalloc/shaped-graph.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/variable.h>

typedef struct jive_reuse_type jive_reuse_type;
typedef struct jive_reuse_output jive_reuse_output;
typedef struct jive_reuse_output jive_reuse;
typedef struct jive_reuse_gate jive_reuse_gate;

extern const jive_type_class JIVE_REUSE_TYPE;
#define JIVE_DECLARE_REUSE_TYPE(name_, resname) \
	jive_reuse_type name_##_struct(resname); \
	const jive_type * name_ = &name_##_struct

class jive_reuse_type final : public jive_state_type {
public:
	virtual ~jive_reuse_type() noexcept {};

	jive_reuse_type(const jive_resource_name * name) noexcept;

	inline const jive_resource_name * name() const noexcept { return name_; }

private:
	const jive_resource_name * name_;
};

class jive_reuse_input final : public jive_state_input {
public:
	virtual ~jive_reuse_input() noexcept;

	jive_reuse_input(const jive_resource_name * name, struct jive_node * node, size_t index,
		jive_output * origin);

	virtual const jive_reuse_type & type() const noexcept { return type_; }

	inline const jive_resource_name * name() const noexcept { return type_.name(); }

private:
	jive_reuse_type type_;
};

class jive_reuse_output final : public jive_state_output {
public:
	virtual ~jive_reuse_output() noexcept;

	jive_reuse_output(const jive_resource_name * name, jive_node * node, size_t index);

	virtual const jive_reuse_type & type() const noexcept { return type_; }

	inline const jive_resource_name * name() const noexcept { return type_.name(); }

private:
	jive_reuse_type type_;
};

extern const jive_gate_class JIVE_REUSE_GATE;
class jive_reuse_gate final : public jive_state_gate {
public:
	virtual ~jive_reuse_gate() noexcept;

	jive_reuse_gate(const jive_resource_name * name_, jive_graph * graph, const char name[]);

	virtual const jive_reuse_type & type() const noexcept { return type_; }

	inline const jive_resource_name * name() const noexcept { return type_.name(); }

private:
	jive_reuse_type type_;
};

jive_reuse_type::jive_reuse_type(const jive_resource_name * name) noexcept
	: jive_state_type(&JIVE_REUSE_TYPE)
	, name_(name)
{}


jive_reuse_input::jive_reuse_input(const jive_resource_name * name, struct jive_node * node,
	size_t index, jive_output * origin)
	: jive_state_input(node, index, origin)
	, type_(name)
{}

jive_reuse_input::~jive_reuse_input() noexcept {}

jive_reuse_output::jive_reuse_output(const jive_resource_name * name, jive_node * node, size_t index)
	: jive_state_output(node, index)
	, type_(name)
{}

jive_reuse_output::~jive_reuse_output() noexcept {}

jive_reuse_gate::jive_reuse_gate(const jive_resource_name * name, jive_graph * graph,
	const char name_[])
	: jive_state_gate(&JIVE_REUSE_GATE, graph, name_)
	, type_(name)
{}

jive_reuse_gate::~jive_reuse_gate() noexcept {}

static void
jive_reuse_gate_fini_(jive_gate * self_)
{
}

const jive_type *
jive_reuse_gate_get_type_(const jive_gate * self_)
{
	const jive_reuse_gate * self = (const jive_reuse_gate *) self_;
	return &self->type();
}

static jive_type *
jive_reuse_type_copy_(const jive_type * self_)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	
	return new jive_reuse_type(self->name());
}

static void
jive_reuse_type_get_label_(const jive_type * self_, struct jive_buffer * buffer)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", self->name()->name);
	jive_buffer_putstr(buffer, tmp);
}

static jive_input *
jive_reuse_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	return new jive_reuse_input(self->name(), node, index, initial_operand);
}

static jive_output *
jive_reuse_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	return new jive_reuse_output(self->name(), node, index);
}

static jive_gate *
jive_reuse_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_reuse_type * self = (const jive_reuse_type *) self_;
	return new jive_reuse_gate(self->name(), graph, name);
}

const jive_type_class JIVE_REUSE_TYPE = {
	parent : &JIVE_TYPE,
	name : "REUSE",
	fini : jive_state_type_fini_, /* inherit */
	get_label : jive_reuse_type_get_label_, /* inherit */
	create_input : jive_reuse_type_create_input_, /* override */
	create_output : jive_reuse_type_create_output_, /* override */
	create_gate : jive_reuse_type_create_gate_, /* override */
	equals : jive_type_equals_, /* inherit */
	copy : jive_reuse_type_copy_, /* override */
};

const jive_gate_class JIVE_REUSE_GATE = {
	parent : &JIVE_GATE,
	fini : jive_reuse_gate_fini_, /* override */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_reuse_gate_get_type_, /* override */
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
		if (!dynamic_cast<jive_anchor_input*>(input))
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
