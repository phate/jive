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
#include <jive/context.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>

namespace jive {

class instruction_operation final : public operation {
public:
	explicit inline constexpr
	instruction_operation(const jive_instruction_class * icls) noexcept
		: icls_(icls)
	{
	}

	inline const jive_instruction_class * icls() const noexcept
	{
		return icls_;
	}

private:
	const jive_instruction_class * icls_;
};

}

struct jive_immediate_node;

typedef jive::operation_node<jive::instruction_operation> jive_instruction_node;

extern const jive_node_class JIVE_INSTRUCTION_NODE;

jive_node *
jive_instruction_node_create_simple(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * const * operands,
	const int64_t * immediates);

jive_node *
jive_instruction_node_create_extended(
	struct jive_region * region,
	const jive_instruction_class * icls,
	jive_output * const * operands,
	const jive_immediate immediates[]);

JIVE_EXPORTED_INLINE jive_node *
jive_instruction_node_create(
	struct jive_region * region,
	const jive_instruction_class * icls,
	struct jive_output * const * operands,
	const int64_t * immediates)
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

JIVE_EXPORTED_INLINE jive_instruction_node *
jive_instruction_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_INSTRUCTION_NODE)
		return (jive_instruction_node *) node;
	else
		return 0;
}

JIVE_EXPORTED_INLINE struct jive_immediate_node *
jive_instruction_node_get_immediate(
	const jive_instruction_node * node,
	size_t index)
{
	const jive_instruction_class * icls = node->operation().icls();
	jive_input * input = node->inputs[index + icls->ninputs];
	return (struct jive_immediate_node *) input->origin()->node;
}

#endif
