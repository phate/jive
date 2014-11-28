/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/reuse.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>

#include <jive/regalloc/shaped-graph.h>

#include <jive/util/buffer.h>
#include <jive/util/intrusive-hash.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/variable.h>

namespace jive {
namespace reuse {

class type final : public jive::state::type {
public:
	virtual ~type() noexcept {};

	type(const jive_resource_name * name) noexcept;

	inline const jive_resource_name * name() const noexcept { return name_; }

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & type) const noexcept override;

	virtual jive::reuse::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;

private:
	const jive_resource_name * name_;
};

class input final : public jive::state::input {
public:
	virtual ~input() noexcept {};

	input(const jive_resource_name * name, struct jive_node * node, size_t index,
		jive::output * origin);

	virtual const jive::reuse::type & type() const noexcept { return type_; }

	inline const jive_resource_name * name() const noexcept { return type_.name(); }

private:
	input(const input & rhs) = delete;
	input& operator=(const input & rhs) = delete;

	jive::reuse::type type_;
};

class output final : public jive::state::output {
public:
	virtual ~output() noexcept {};

	output(const jive_resource_name * name, jive_node * node, size_t index);

	virtual const jive::reuse::type & type() const noexcept { return type_; }

	inline const jive_resource_name * name() const noexcept { return type_.name(); }

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::reuse::type type_;
};

class gate final : public jive::state::gate {
public:
	virtual ~gate() noexcept {};

	gate(const jive_resource_name * name_, jive_graph * graph, const char name[]);

	virtual const jive::reuse::type & type() const noexcept { return type_; }

	inline const jive_resource_name * name() const noexcept { return type_.name(); }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::reuse::type type_;
};

type::type(const jive_resource_name * name) noexcept
	: jive::state::type()
	, name_(name)
{}

std::string
type::debug_string() const
{
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "reuse %s", name()->name);
	return std::string(tmp);
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::reuse::type*>(&other) != nullptr;
}

jive::reuse::type *
type::copy() const
{
	return new jive::reuse::type(this->name());
}

jive::input *
type::create_input(jive_node * node, size_t index, jive::output * origin) const
{
	return new jive::reuse::input(name(), node, index, origin);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::reuse::output(name(), node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::reuse::gate(this->name(), graph, name);
}

input::input(const jive_resource_name * name, struct jive_node * node, size_t index,
	jive::output * origin)
	: jive::state::input(node, index, origin)
	, type_(name)
{}

output::output(const jive_resource_name * name, jive_node * node, size_t index)
	: jive::state::output(node, index)
	, type_(name)
{}

gate::gate(const jive_resource_name * name, jive_graph * graph, const char name_[])
	: jive::state::gate(graph, name_)
	, type_(name)
{}

}
}

/* structures for tracking current active set and users */

typedef struct jive_used_name jive_used_name;
struct jive_used_name {
	const jive_resource_name * name;
	std::vector<jive_node*> read;
	std::vector<jive_node*> clobber;
	size_t read_count, write_count;
	struct {
		jive_used_name * prev;
		jive_used_name * next;
	} used_names_list;
private:
	jive::detail::intrusive_hash_anchor<jive_used_name> hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		const jive_resource_name *,
		jive_used_name,
		&jive_used_name::name,
		&jive_used_name::hash_chain
	> hash_chain_accessor;
};

typedef jive::detail::intrusive_hash<
	const jive_resource_name *,
	jive_used_name,
	jive_used_name::hash_chain_accessor
> jive_used_name_hash;

typedef struct jive_names_use jive_names_use;
struct jive_names_use {
	jive_used_name_hash hash;
	struct {
		jive_used_name * first;
		jive_used_name * last;
	} list;
};

static void
jive_names_use_init(jive_names_use * self)
{
	self->list.first = self->list.last = 0;
}

static jive_used_name *
jive_names_use_lookup(jive_names_use * self, const jive_resource_name * name)
{
	JIVE_DEBUG_ASSERT(name);
	
	jive_used_name * used_name;
	auto i = self->hash.find(name);
	if (i == self->hash.end()) {
		used_name = new jive_used_name;
		used_name->name = name;
		used_name->read_count = 0;
		used_name->write_count = 0;
		JIVE_LIST_PUSH_BACK(self->list, used_name, used_names_list);
		JIVE_DEBUG_ASSERT(self->hash.find(name) == self->hash.end());
		self->hash.insert(used_name);
	} else
		used_name = i.ptr();
	
	return used_name;
}

static void
jive_names_use_fini(jive_names_use * self)
{
	while (self->list.first) {
		jive_used_name * used_name = self->list.first;
		self->hash.erase(used_name);
		JIVE_LIST_REMOVE(self->list, used_name, used_names_list);
		delete used_name;
	}
}

static void
jive_names_use_read(jive_names_use * self, const jive_resource_name * name, jive_node * node)
{
	jive_used_name * used_name = jive_names_use_lookup(self, name);
	
	used_name->read.push_back(node);
	used_name->read_count ++;
}

static void
jive_names_use_clobber(jive_names_use * self, const jive_resource_name * name, jive_node * node)
{
	jive_used_name * used_name = jive_names_use_lookup(self, name);
	
	jive::reuse::type type(name);
	size_t n;
	for (n = 0; n < used_name->read.size(); n++) {
		jive_node * before = used_name->read[n];
		
		if (node != before)
			jive_node_add_input(node, &type, jive_node_add_output(before, &type));
	}
	
	used_name->clobber.push_back(node);
	used_name->write_count ++;
}

static void
jive_names_use_write(jive_names_use * self, const jive_resource_name * name, jive_node * node)
{
	jive_used_name * used_name = jive_names_use_lookup(self, name);
	
	jive::reuse::type type(name);
	
	size_t n;
	for (n = 0; n < used_name->read.size(); n++) {
		jive_node * before = used_name->read[n];
		if (node != before)
			jive_node_add_input(node, &type, jive_node_add_output(before, &type));
	}
	for (n = 0; n < used_name->clobber.size(); n++) {
		jive_node * before = used_name->clobber[n];
		if (node != before)
			jive_node_add_input(node, &type, jive_node_add_output(before, &type));
	}

	used_name->read.clear();
	used_name->clobber.clear();
	used_name->write_count ++;
}

/* add reuse edges to graph */

static void
jive_regalloc_reuse_record_region(jive_shaped_graph * shaped_graph, jive_region * region,
	jive_names_use * names_use);

static void
jive_regalloc_reuse_record_node(jive_shaped_graph * shaped_graph, jive_node * node,
	jive_names_use * names_use)
{
	jive_names_use sub_names_use;
	jive_names_use_init(&sub_names_use);
	jive_used_name * used_name;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (!dynamic_cast<jive::achr::input*>(input))
			continue;
		
		jive_names_use inner_names_use;
		jive_names_use_init(&inner_names_use);
		
		jive_regalloc_reuse_record_region(shaped_graph, input->producer()->region, &inner_names_use);
		
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
		jive::output * output = node->outputs[n];
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
jive_regalloc_reuse_record_region(jive_shaped_graph * shaped_graph, jive_region * region,
	jive_names_use * names_use)
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
	
	jive_names_use names_use;
	jive_names_use_init(&names_use);
	
	jive_regalloc_reuse_record_region(shaped_graph, graph->root_region, &names_use);
	
	jive_names_use_fini(&names_use);
}
