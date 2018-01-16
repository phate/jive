/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_CONTROL_H
#define JIVE_RVSDG_CONTROL_H

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/unary.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/strfmt.h>

#include <unordered_map>

#include <inttypes.h>

namespace jive {
namespace ctl {

/* control type */

class type final : public jive::statetype {
public:
	virtual
	~type() noexcept;

	type(size_t nalternatives);

	virtual std::string
	debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

	inline size_t
	nalternatives() const noexcept
	{
		return nalternatives_;
	}

private:
	size_t nalternatives_;
};

}

static inline bool
is_ctltype(const jive::type & type) noexcept
{
	return dynamic_cast<const jive::ctl::type*>(&type) != nullptr;
}

/* control value representation */

namespace ctl {

class value_repr {
public:
	value_repr(size_t alternative, size_t nalternatives);

	inline bool
	operator==(const jive::ctl::value_repr & other) const noexcept
	{
		return alternative_ == other.alternative_ && nalternatives_ == other.nalternatives_;
	}

	inline bool
	operator!=(const jive::ctl::value_repr & other) const noexcept
	{
		return !(*this == other);
	}

	inline size_t
	alternative() const noexcept
	{
		return alternative_;
	}

	inline size_t
	nalternatives() const noexcept
	{
		return nalternatives_;
	}

private:
	size_t alternative_;
	size_t nalternatives_;
};

/* control constant */

struct type_of_value {
	type operator()(const value_repr & repr) const
	{
		return type(repr.nalternatives());
	}
};

struct format_value {
	std::string operator()(const value_repr & repr) const
	{
		return jive::detail::strfmt("CTL(", repr.alternative(), ")");
	}
};

typedef domain_const_op<type, value_repr, format_value, type_of_value> constant_op;

}

static inline bool
is_ctlconstant_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::ctl::constant_op*>(&op) != nullptr;
}

static inline const ctl::constant_op &
to_ctlconstant_op(const jive::operation & op) noexcept
{
	JIVE_DEBUG_ASSERT(is_ctlconstant_op(op));
	return *static_cast<const ctl::constant_op*>(&op);
}

/* match operator */

namespace ctl {

class match_op final : public jive::unary_op {
	typedef std::unordered_map<uint64_t,uint64_t>::const_iterator const_iterator;

public:
	virtual
	~match_op() noexcept;

	match_op(
		size_t nbits,
		const std::unordered_map<uint64_t, uint64_t> & mapping,
		uint64_t default_alternative,
		size_t nalternatives);

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(jive_unop_reduction_path_t path, jive::output * arg) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	inline uint64_t
	nalternatives() const noexcept
	{
		return static_cast<const jive::ctl::type*>(&result_.type())->nalternatives();
	}

	inline uint64_t
	alternative(uint64_t value) const noexcept
	{
		auto it = mapping_.find(value);
		if (it != mapping_.end())
			return it->second;

		return default_alternative_;
	}

	inline uint64_t
	default_alternative() const noexcept
	{
		return default_alternative_;
	}

	inline size_t
	nbits() const noexcept
	{
		return static_cast<const bittype*>(&argument_.type())->nbits();
	}

	inline const_iterator
	begin() const
	{
		return mapping_.begin();
	}

	inline const_iterator
	end() const
	{
		return mapping_.end();
	}

private:
	jive::port result_;
	jive::port argument_;
	uint64_t default_alternative_;
	std::unordered_map<uint64_t, uint64_t> mapping_;
};

jive::output *
match(
	size_t nbits,
	const std::unordered_map<uint64_t, uint64_t> & mapping,
	uint64_t default_alternative,
	size_t nalternatives,
	jive::output * operand);

const type boolean(2);

}

// declare explicit instantiation
extern template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;

static inline bool
is_match_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::ctl::match_op*>(&op) != nullptr;
}

static inline bool
is_match_node(const jive::node * node) noexcept
{
	return is_opnode<jive::ctl::match_op>(node);
}

static inline const ctl::match_op &
to_match_op(const jive::operation & op) noexcept
{
	JIVE_DEBUG_ASSERT(is_match_op(op));
	return *static_cast<const ctl::match_op*>(&op);
}

}

jive::output *
jive_control_constant(jive::region * region, size_t nalternatives, size_t alternative);

static inline jive::output *
jive_control_false(jive::region * region)
{
	return jive_control_constant(region, 2, 0);
}

static inline jive::output *
jive_control_true(jive::region * region)
{
	return jive_control_constant(region, 2, 1);
}

#endif
