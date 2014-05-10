/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESS_TRANSFORM_H
#define JIVE_ARCH_ADDRESS_TRANSFORM_H

#include <jive/arch/address.h>
#include <jive/arch/call.h>
#include <jive/arch/load.h>
#include <jive/common.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>

namespace jive {

class address_to_bitstring_operation final : public unary_operation {
public:
	inline
	address_to_bitstring_operation(
		size_t nbits,
		const jive_type * original_type)
		: nbits_(nbits)
		, original_type_(jive_type_copy(original_type))
	{
	}

	inline
	address_to_bitstring_operation(
		address_to_bitstring_operation && other) noexcept = default;

	inline
	address_to_bitstring_operation(
		const address_to_bitstring_operation & other)
		: nbits_(other.nbits_)
		, original_type_(jive_type_copy(other.original_type_.get()))
	{
	}

	inline size_t nbits() const noexcept { return nbits_; }
	inline const jive_type & original_type() const noexcept { return *original_type_; }

private:
	size_t nbits_;
	std::unique_ptr<jive_type> original_type_;
};

class bitstring_to_address_operation final : public unary_operation {
public:
	inline
	bitstring_to_address_operation(
		size_t nbits,
		const jive_type * original_type)
		: nbits_(nbits)
		, original_type_(jive_type_copy(original_type))
	{
	}

	inline
	bitstring_to_address_operation(
		bitstring_to_address_operation && other) noexcept = default;

	inline
	bitstring_to_address_operation(
		const bitstring_to_address_operation & other)
		: nbits_(other.nbits_)
		, original_type_(jive_type_copy(other.original_type_.get()))
	{
	}

	inline size_t nbits() const noexcept { return nbits_; }
	inline const jive_type & original_type() const noexcept { return *original_type_; }

private:
	size_t nbits_;
	std::unique_ptr<jive_type> original_type_;
};

}

typedef jive::operation_node<jive::address_to_bitstring_operation>
	jive_address_to_bitstring_node;

typedef jive::operation_node<jive::bitstring_to_address_operation>
	jive_bitstring_to_address_node;

struct jive_memlayout_mapper;

/* address_to_bitstring node */

extern const jive_unary_operation_class JIVE_ADDRESS_TO_BITSTRING_NODE_;
#define JIVE_ADDRESS_TO_BITSTRING_NODE (JIVE_ADDRESS_TO_BITSTRING_NODE_.base)

jive_node *
jive_address_to_bitstring_node_create(struct jive_region * region,
	jive_output * address, size_t nbits, const jive_type * original_type);

jive_output *
jive_address_to_bitstring_create(jive_output * address, size_t nbits,
	const jive_type * original_type);

JIVE_EXPORTED_INLINE jive_address_to_bitstring_node *
jive_address_to_bitstring_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_ADDRESS_TO_BITSTRING_NODE))
		return (jive_address_to_bitstring_node *) node;
	else
		return 0;
}

/* bitstring_to_address node */

extern const jive_unary_operation_class JIVE_BITSTRING_TO_ADDRESS_NODE_;
#define JIVE_BITSTRING_TO_ADDRESS_NODE (JIVE_BITSTRING_TO_ADDRESS_NODE_.base)

jive_node *
jive_bitstring_to_address_node_create(struct jive_region * region,
	jive_output * bitstring, size_t nbits, const jive_type * original_type);

jive_output *
jive_bitstring_to_address_create(jive_output * bitstring, size_t nbits,
	const jive_type * original_type);

JIVE_EXPORTED_INLINE jive_bitstring_to_address_node *
jive_bitstring_to_address_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_BITSTRING_TO_ADDRESS_NODE))
		return (jive_bitstring_to_address_node *) node;
	else
		return 0;
}

/* reductions */

struct jive_store_node;

void
jive_load_node_address_transform(jive_load_node * node,
	size_t nbits);

void
jive_store_node_address_transform(struct jive_store_node * node,
	size_t nbits);

void
jive_label_to_address_node_address_transform(jive_label_to_address_node * node,
	size_t nbits);

void
jive_call_node_address_transform(jive_call_node * node,
	size_t nbits);

void
jive_lambda_node_address_transform(const jive_lambda_node * node, size_t nbits);

void
jive_apply_node_address_transform(const jive_apply_node * node, size_t nbits);

void
jive_memberof_node_address_transform(jive_memberof_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_containerof_node_address_transform(jive_containerof_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_arraysubscript_node_address_transform(jive_arraysubscript_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_arrayindex_node_address_transform(jive_arrayindex_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_graph_address_transform(jive_graph * graph,
	struct jive_memlayout_mapper * mapper);

#endif
