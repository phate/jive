/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_NORMAL_FORM_H
#define JIVE_VSDG_NODE_NORMAL_FORM_H

#include <stddef.h>

#include <vector>

#include <jive/common.h>
#include <jive/util/list.h>

/* normal forms */

class jive_node;
struct jive_graph;
struct jive_node_class;

struct jive_node_normal_form_class;

namespace jive {

class operation;
class output;

class node_normal_form {
public:
	virtual
	~node_normal_form() noexcept;

	inline
	node_normal_form(
		const jive_node_class * node_class,
		jive::node_normal_form * parent,
		jive_graph * graph) noexcept
		: node_class(node_class)
		, parent_(parent)
		, graph_(graph)
		, enable_mutable_(true)
		, enable_cse_(true)
	{
		if (parent) {
			enable_mutable_ = parent->enable_mutable_;
			enable_cse_ = parent->enable_cse_;
		}
		subclasses.first = subclasses.last = nullptr;
	}

	virtual bool
	normalize_node(jive_node * node) const;

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::output *> & arguments) const;

	virtual std::vector<jive::output *>
	normalized_create(
		const jive::operation & op,
		const std::vector<jive::output *> & arguments) const;

	inline node_normal_form *
	parent() const noexcept { return parent_; }
	inline jive_graph *
	graph() const noexcept { return graph_; }

	virtual void
	set_mutable(bool enable);
	inline bool
	get_mutable() const noexcept { return enable_mutable_; }

	virtual void
	set_cse(bool enable);
	inline bool
	get_cse() const noexcept { return enable_cse_; }

	const jive_node_normal_form_class * class_;
	const jive_node_class * node_class;
	
	struct {
		node_normal_form * first;
		node_normal_form * last;
	} subclasses;
	struct {
		node_normal_form * prev;
		node_normal_form * next;
	} normal_form_subclass_list;
	struct {
		node_normal_form * prev;
		node_normal_form * next;
	} hash_chain;

protected:
	template<typename X, void(X::*fn)(bool)>
	void
	children_set(bool enable)
	{
		jive::node_normal_form * child;
		JIVE_LIST_ITERATE(subclasses, child, normal_form_subclass_list) {
			if (auto nf = dynamic_cast<X *>(child)) {
				(nf->*fn)(enable);
			}
		}
	}

private:
	node_normal_form * parent_;
	jive_graph * graph_;

	bool enable_mutable_;
	bool enable_cse_;
};

}

struct jive_node_normal_form_class {
	const jive_node_normal_form_class * parent;
	void (*fini)(jive::node_normal_form * self);
	/* return true, if normalized already */
	bool (*normalize_node)(const jive::node_normal_form * self, jive_node * node);
	/* return true, if normalized already */
	bool (*operands_are_normalized)(
		const jive::node_normal_form * self,
		size_t noperands, jive::output * const operands[],
		const jive::operation * attrs);
	void (*normalized_create)(const jive::node_normal_form * self, struct jive_graph * graph,
		const jive::operation * attrs, size_t noperands, jive::output * const operands[],
		jive::output * results[]);
	void (*set_mutable)(jive::node_normal_form * self, bool enable);
	void (*set_cse)(jive::node_normal_form * self, bool enable);
};

extern const jive_node_normal_form_class JIVE_NODE_NORMAL_FORM;

#endif
