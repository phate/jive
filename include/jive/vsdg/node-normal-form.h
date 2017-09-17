/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_NORMAL_FORM_H
#define JIVE_VSDG_NODE_NORMAL_FORM_H

#include <stddef.h>

#include <typeindex>
#include <typeinfo>
#include <vector>

#include <jive/common.h>
#include <jive/util/intrusive-hash.h>
#include <jive/util/list.h>

/* normal forms */

namespace jive {

class graph;
class node;
class operation;
class output;
class region;

class node_normal_form {
public:
	virtual
	~node_normal_form() noexcept;

	inline
	node_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph) noexcept
		: operator_class_(operator_class)
		, parent_(parent)
		, graph_(graph)
		, enable_mutable_(true)
	{
		subclasses_.first = subclasses_.last = nullptr;
		if (parent) {
			enable_mutable_ = parent->enable_mutable_;
			JIVE_LIST_PUSH_BACK(parent->subclasses_, this, normal_form_subclass_list_);
		}
	}

	virtual bool
	normalize_node(jive::node * node) const;

	inline node_normal_form *
	parent() const noexcept { return parent_; }
	inline jive::graph *
	graph() const noexcept { return graph_; }

	virtual void
	set_mutable(bool enable);
	inline bool
	get_mutable() const noexcept { return enable_mutable_; }

	static void
	register_factory(
		const std::type_info & type,
		jive::node_normal_form *(*fn)(
			const std::type_info & operator_class,
			jive::node_normal_form * parent,
			jive::graph * graph));

	static node_normal_form *
	create(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph);

	class opclass_hash_accessor {
	public:
		std::type_index
		get_key(const node_normal_form * obj) const noexcept
		{
			return std::type_index(obj->operator_class_);
		}
		node_normal_form *
		get_prev(const node_normal_form * obj) const noexcept
		{
			return obj->new_hash_chain.prev;
		}
		void
		set_prev(node_normal_form * obj, node_normal_form * prev) const noexcept
		{
			obj->new_hash_chain.prev = prev;
		}
		node_normal_form *
		get_next(const node_normal_form * obj) const noexcept
		{
			return obj->new_hash_chain.next;
		}
		void
		set_next(node_normal_form * obj, node_normal_form * next) const noexcept
		{
			obj->new_hash_chain.next = next;
		}
	};

protected:
	template<typename X, void(X::*fn)(bool)>
	void
	children_set(bool enable)
	{
		jive::node_normal_form * child;
		JIVE_LIST_ITERATE(subclasses_, child, normal_form_subclass_list_) {
			if (auto nf = dynamic_cast<X *>(child)) {
				(nf->*fn)(enable);
			}
		}
	}

private:
	const std::type_info & operator_class_;
	node_normal_form * parent_;
	jive::graph * graph_;

	struct {
		node_normal_form * prev;
		node_normal_form * next;
	} new_hash_chain;

	bool enable_mutable_;

	struct {
		node_normal_form * first;
		node_normal_form * last;
	} subclasses_;

	struct {
		node_normal_form * prev;
		node_normal_form * next;
	} normal_form_subclass_list_;
};

typedef jive::detail::owner_intrusive_hash<
	std::type_index,
	jive::node_normal_form,
	jive::node_normal_form::opclass_hash_accessor> node_normal_form_hash;

}

#endif
