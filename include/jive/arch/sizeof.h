/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SIZEOF_H
#define JIVE_ARCH_SIZEOF_H

#include <memory>

#include <jive/vsdg/node.h>
#include <jive/vsdg/nullary.h>

namespace jive {

class sizeof_op final : public base::nullary_op {
public:
	virtual
	~sizeof_op() noexcept;

	inline explicit
	sizeof_op(const jive::valuetype & type)
	: type_(type.copy())
	{}

	inline
	sizeof_op(const sizeof_op & other)
		: type_(other.type().copy())
	{
	}

	inline
	sizeof_op(sizeof_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::valuetype &
	type() const noexcept
	{
		return *static_cast<const valuetype*>(type_.get());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<jive::type> type_;
};

class memlayout_mapper;

}

jive::output *
jive_sizeof_create(jive::region * region, const jive::valuetype * type);

void
jive_sizeof_node_reduce(const jive::node * node, jive::memlayout_mapper * mapper);

#endif
