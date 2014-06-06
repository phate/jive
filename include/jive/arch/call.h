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
		const std::vector<std::unique_ptr<jive::base::type>> & return_types);

	inline
	call_operation(
		const jive_calling_convention * calling_convention,
		std::vector<std::unique_ptr<jive::base::type>> && return_types) noexcept
		: calling_convention_(calling_convention)
		, return_types_(std::move(return_types))
	{
	}

	call_operation(const call_operation & other);

	call_operation(call_operation && other) = default;

	inline const jive_calling_convention *
	calling_convention() const noexcept { return calling_convention_; }

	inline const std::vector<std::unique_ptr<jive::base::type>> &
	return_types() const noexcept { return return_types_; }
private:
	const jive_calling_convention * calling_convention_;
	std::vector<std::unique_ptr<jive::base::type>> return_types_;
};

}

extern const jive_node_class JIVE_CALL_NODE;

typedef jive::operation_node<jive::call_operation> jive_call_node;

struct jive_node *
jive_call_by_address_node_create(struct jive_region * region,
	struct jive_output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[]);

struct jive_output * const *
jive_call_by_address_create(struct jive_output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[]);

struct jive_node *
jive_call_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[]);

struct jive_output * const *
jive_call_by_bitstring_create(struct jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const jive::base::type * const return_types[]);

JIVE_EXPORTED_INLINE jive_call_node *
jive_call_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_CALL_NODE))
		return (jive_call_node *) node;
	else
		return NULL;
}

#endif
