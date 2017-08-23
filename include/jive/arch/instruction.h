/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTION_H
#define JIVE_ARCH_INSTRUCTION_H

#include <algorithm>
#include <string.h>

#include <jive/arch/immediate-node.h>
#include <jive/arch/immediate-type.h>
#include <jive/arch/instruction-class.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/ptr-collection.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/nullary.h>
#include <jive/vsdg/statetype.h>

namespace jive {

class instruction_op final : public simple_op {
public:
	virtual
	~instruction_op() noexcept;

	inline
	instruction_op(
		const jive::instruction_class * icls,
		const std::vector<const jive::state::type*> & istates,
		const std::vector<const jive::state::type*> & ostates)
	: icls_(icls)
	{
		static const jive::imm::type immtype;
		for (size_t n = 0; n < icls->ninputs(); n++)
			arguments_.push_back(icls->input(n));
		for (size_t n = 0; n < icls->nimmediates(); n++)
			arguments_.push_back(immtype);
		for (const auto & type : istates)
			arguments_.push_back({std::move(type->copy())});

		for (size_t n = 0; n < icls->noutputs(); n++)
			results_.push_back({icls->output(n)});
		for (const auto & type : ostates)
			results_.push_back({std::move(type->copy())});
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual const jive::resource_class *
	result_cls(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::instruction_class *
	icls() const noexcept
	{
		return icls_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<jive::port> results_;
	std::vector<jive::port> arguments_;
	const jive::instruction_class * icls_;
};

}

jive::node *
jive_instruction_node_create_simple(
	struct jive::region * region,
	const jive::instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates);

jive::node *
jive_instruction_node_create_extended(
	struct jive::region * region,
	const jive::instruction_class * icls,
	jive::output * const * operands,
	const jive::immediate immediates[]);

jive::node *
jive_instruction_node_create(
	struct jive::region * region,
	const jive::instruction_class * icls,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::immediate> & immediates,
	const std::vector<const jive::state::type*> & itypes,
	const std::vector<jive::output*> & istates,
	const std::vector<const jive::state::type*> & otypes);

static inline jive::node *
jive_instruction_node_create(
	struct jive::region * region,
	const jive::instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates)
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

#endif
