/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <string.h>

#include <jive/arch/immediate-node.h>
#include <jive/arch/instruction-class.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>

namespace jive {

class instruction_op final : public operation {
public:
	virtual
	~instruction_op() noexcept;

	explicit inline constexpr
	instruction_op(const jive_instruction_class * icls) noexcept
		: icls_(icls)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline const jive_instruction_class * icls() const noexcept
	{
		return icls_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive_instruction_class * icls_;
};

}

jive_node *
jive_instruction_node_create_simple(
	struct jive_region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates);

jive_node *
jive_instruction_node_create_extended(
	struct jive_region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const jive_immediate immediates[]);

JIVE_EXPORTED_INLINE jive_node *
jive_instruction_node_create(
	struct jive_region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates)
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

JIVE_EXPORTED_INLINE jive_immediate
jive_instruction_node_get_immediate(
	const jive_node * node,
	size_t index)
{
	const jive_instruction_class * icls =
		static_cast<const jive::instruction_op &>(node->operation()).icls();
	jive::input * input = node->inputs[index + icls->ninputs];
	return static_cast<const jive::immediate_op &>(
		input->origin()->node()->operation()).value();
}

#endif
