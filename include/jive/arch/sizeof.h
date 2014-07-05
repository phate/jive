/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SIZEOF_H
#define JIVE_ARCH_SIZEOF_H

#include <memory>

#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/valuetype.h>

struct jive_memlayout_mapper;

namespace jive {

class sizeof_op final : public base::nullary_op {
public:
	virtual
	~sizeof_op() noexcept;

	inline explicit
	sizeof_op(const jive::value::type & type)
		: type_(type.copy())
	{
	}

	inline
	sizeof_op(const sizeof_op & other)
		: type_(other.type().copy())
	{
	}

	inline
	sizeof_op(sizeof_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline const jive::value::type & type() const noexcept { return *type_; }

private:
	std::unique_ptr<jive::value::type> type_;
};

}

typedef jive::operation_node<jive::sizeof_op> jive_sizeof_node;

extern const jive_node_class JIVE_SIZEOF_NODE;

jive::output *
jive_sizeof_create(jive_region * region, const jive::value::type * type);

void
jive_sizeof_node_reduce(const jive_sizeof_node * node, jive_memlayout_mapper * mapper);

#endif
