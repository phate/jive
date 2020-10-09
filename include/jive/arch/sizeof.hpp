/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SIZEOF_HPP
#define JIVE_ARCH_SIZEOF_HPP

#include <memory>

#include <jive/rvsdg/node.hpp>
#include <jive/rvsdg/nullary.hpp>
#include <jive/types/bitstring.hpp>

namespace jive {

class sizeof_op final : public nullary_op {
public:
	virtual
	~sizeof_op() noexcept;

	inline explicit
	sizeof_op(const jive::valuetype & type)
	/* FIXME: either need a "universal" integer type,
	or some way to specify the representation type for the
	sizeof operator */
	: nullary_op(bittype(32))
	, type_(type.copy())
	{}

	inline
	sizeof_op(const sizeof_op & other)
	: nullary_op(other)
	, type_(other.type().copy())
	{}

	inline
	sizeof_op(sizeof_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

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
