/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CALL_H
#define JIVE_ARCH_CALL_H

#include <memory>
#include <vector>

#include <jive/common.h>
#include <jive/vsdg/node.h>

struct jive_context;

namespace jive {
	class type;
}

/* FIXME: opaque type for now -- to be filled in later */
struct jive_calling_convention;

namespace jive {

class call_operation final : public operation {
public:
	virtual ~call_operation() noexcept;

	call_operation(
		const jive_calling_convention * calling_convention,
		const std::vector<std::unique_ptr<jive::base::type>> & argument_types,
		const std::vector<std::unique_ptr<jive::base::type>> & result_types);

	inline
	call_operation(
		const jive_calling_convention * calling_convention,
		std::vector<std::unique_ptr<jive::base::type>> && argument_types,
		std::vector<std::unique_ptr<jive::base::type>> && result_types) noexcept
		: calling_convention_(calling_convention)
		, argument_types_(std::move(argument_types))
		, result_types_(std::move(result_types))
	{
	}

	call_operation(const call_operation & other);

	call_operation(call_operation && other) = default;

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

	inline const jive_calling_convention *
	calling_convention() const noexcept { return calling_convention_; }

	inline const std::vector<std::unique_ptr<jive::base::type>> &
	argument_types() const noexcept { return argument_types_; }

	inline const std::vector<std::unique_ptr<jive::base::type>> &
	result_types() const noexcept { return result_types_; }

private:
	const jive_calling_convention * calling_convention_;
	std::vector<std::unique_ptr<jive::base::type>> argument_types_;
	std::vector<std::unique_ptr<jive::base::type>> result_types_;
};

}

extern const jive_node_class JIVE_CALL_NODE;

typedef jive::operation_node<jive::call_operation> jive_call_node;

struct jive_node *
jive_call_by_address_node_create(struct jive_region * region,
	jive::output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

jive::output * const *
jive_call_by_address_create(jive::output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

struct jive_node *
jive_call_by_bitstring_node_create(struct jive_region * region,
	jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

jive::output * const *
jive_call_by_bitstring_create(jive::output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive::output * const arguments[],
	size_t nreturns, const jive::base::type * const result_types[]);

JIVE_EXPORTED_INLINE jive_call_node *
jive_call_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_CALL_NODE))
		return (jive_call_node *) node;
	else
		return NULL;
}

#endif
