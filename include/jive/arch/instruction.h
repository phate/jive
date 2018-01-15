/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <algorithm>
#include <string.h>

#include <jive/arch/immediate.h>
#include <jive/arch/instruction-class.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/ptr-collection.h>

namespace jive {

class instruction_op final : public simple_op {
public:
	virtual
	~instruction_op() noexcept;

	inline
	instruction_op(
		const jive::instruction * icls,
		const std::vector<jive::port> & iports,
		const std::vector<jive::port> & oports)
	: icls_(icls)
	{
		static const immtype it;
		for (size_t n = 0; n < icls->ninputs(); n++)
			arguments_.push_back(icls->input(n));
		for (size_t n = 0; n < icls->nimmediates(); n++)
			arguments_.push_back(it);
		for (const auto & port : iports)
			arguments_.push_back(port);

		for (size_t n = 0; n < icls->noutputs(); n++)
			results_.push_back({icls->output(n)});
		for (const auto & port : oports)
			results_.push_back(port);
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::instruction *
	icls() const noexcept
	{
		return icls_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::instruction * icls_;
	std::vector<jive::port> results_;
	std::vector<jive::port> arguments_;
};

static inline jive::node *
create_instruction(
	jive::region * region,
	const jive::instruction * icls,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::port> & iports,
	const std::vector<jive::port> & oports)
{
	jive::instruction_op op(icls, iports, oports);
	return region->add_simple_node(op, operands);
}

static inline jive::node *
create_instruction(
	jive::region * region,
	const jive::instruction * icls,
	const std::vector<jive::output*> & operands)
{
	return create_instruction(region, icls, operands, {}, {});
}

}

#endif
