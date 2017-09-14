/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_CONTROL_H
#define JIVE_VSDG_CONTROL_H

#include <jive/types/bitstring/type.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/nullary.h>
#include <jive/vsdg/unary.h>

#include <unordered_map>

#include <inttypes.h>

namespace jive {
namespace ctl {

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

typedef base::domain_const_op<
	type, value_repr, format_value, type_of_value
> constant_op;

class match_op final : public base::unary_op {
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

}

namespace base {
// declare explicit instantiation
extern template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;
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
