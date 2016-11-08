/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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
#include <jive/util/ptr-collection.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/statetype.h>

namespace jive {

class instruction_op final : public operation {
public:
	virtual
	~instruction_op() noexcept;

	inline
	instruction_op(
		const jive_instruction_class * icls,
		const std::vector<std::unique_ptr<jive::state::type>> & istates,
		const std::vector<std::unique_ptr<jive::state::type>> & ostates)
		: icls_(icls)
		, istates_(detail::unique_ptr_vector_copy(istates))
		, ostates_(detail::unique_ptr_vector_copy(ostates))
	{}

	inline
	instruction_op(const instruction_op & other)
		: icls_(other.icls_)
		, istates_(detail::unique_ptr_vector_copy(other.istates_))
		, ostates_(detail::unique_ptr_vector_copy(other.ostates_))
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive_resource_class *
	argument_cls(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual const jive_resource_class *
	result_cls(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	inline const jive_instruction_class * icls() const noexcept
	{
		return icls_;
	}

	inline const std::vector<std::unique_ptr<jive::state::type>> &
	istates() const noexcept
	{
		return istates_;
	}

	inline const std::vector<std::unique_ptr<jive::state::type>> &
	ostates() const noexcept
	{
		return ostates_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive_instruction_class * icls_;
	std::vector<std::unique_ptr<jive::state::type>> istates_;
	std::vector<std::unique_ptr<jive::state::type>> ostates_;
};

}

jive_node *
jive_instruction_node_create_simple(
	struct jive::region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates);

jive_node *
jive_instruction_node_create_extended(
	struct jive::region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const jive::immediate immediates[]);

jive_node *
jive_instruction_node_create(
	struct jive::region * region,
	const jive_instruction_class * icls,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::immediate> & immediates,
	const std::vector<const jive::state::type*> & itypes,
	const std::vector<jive::output*> & istates,
	const std::vector<const jive::state::type*> & otypes);

JIVE_EXPORTED_INLINE jive_node *
jive_instruction_node_create(
	struct jive::region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates)
{
	return jive_instruction_node_create_simple(region, icls, operands, immediates);
}

#endif
