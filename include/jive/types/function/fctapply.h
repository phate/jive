/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTAPPLY_H
#define JIVE_TYPES_FUNCTION_FCTAPPLY_H

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/function/fcttype.h>

namespace jive {

class apply_op final : public jive::simple_op {
public:
	virtual
	~apply_op() noexcept;

	inline
	apply_op(const fcttype & type)
	: simple_op(create_operands(type), create_results(type))
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const fcttype &
	function_type() const noexcept
	{
		return *static_cast<const jive::fcttype*>(&argument(0).type());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	static std::vector<jive::port>
	create_operands(const fcttype & type);

	static std::vector<jive::port>
	create_results(const fcttype & type);
};

static inline std::vector<jive::output*>
create_apply(jive::output * function, const std::vector<jive::output*> & arguments)
{
	auto ft = dynamic_cast<const fcttype*>(&function->type());
	if (!ft) throw type_error("fct", function->type().debug_string());

	apply_op op(*ft);
	std::vector<jive::output*> operands({function});
	operands.insert(operands.end(), arguments.begin(), arguments.end());

	return simple_node::create_normalized(function->region(), op, operands);
}

}

#endif
