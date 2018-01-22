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
		const jive::instruction * i,
		const std::vector<jive::port> & iports,
		const std::vector<jive::port> & oports)
	: simple_op(create_operands(i, iports), create_results(i, oports))
	, icls_(i)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

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
	static std::vector<jive::port>
	create_operands(
		const jive::instruction * icls,
		const std::vector<jive::port> & iports);

	static std::vector<jive::port>
	create_results(
		const jive::instruction * icls,
		const std::vector<jive::port> & oprts);

	const jive::instruction * icls_;
};

static inline bool
is_instruction_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const instruction_op*>(&op);
}

static inline bool
is_instruction_node(const jive::node * node) noexcept
{
	return is_opnode<instruction_op>(node);
}

static inline jive::node *
create_instruction(
	jive::region * region,
	const jive::instruction * icls,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::port> & iports,
	const std::vector<jive::port> & oports)
{
	jive::instruction_op op(icls, iports, oports);
	return simple_node::create(region, op, operands);
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
