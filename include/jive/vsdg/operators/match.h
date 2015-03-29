/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_MATCH_H
#define JIVE_VSDG_OPERATORS_MATCH_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/operators/unary.h>

#include <unordered_map>

namespace jive {

class match_op final : public base::unary_op {
public:
	virtual
	~match_op() noexcept;

	match_op(const jive::bits::type & type, const std::vector<size_t> & constants);

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

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
		return mapping_.size()+1;
	}

private:
	jive::ctl::type otype_;
	jive::bits::type itype_;
	std::unordered_map<uint64_t, uint64_t> mapping_;
};

}

#endif
