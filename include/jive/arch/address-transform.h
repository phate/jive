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
#include <jive/arch/store.h>
#include <jive/common.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>

namespace jive {

class address_to_bitstring_operation final : public base::unary_op {
public:
	virtual ~address_to_bitstring_operation() noexcept;

	inline
	address_to_bitstring_operation(
		size_t nbits,
		const jive::base::type * original_type)
		: result_type_(nbits)
		, original_type_(original_type->copy())
	{
	}

	inline
	address_to_bitstring_operation(
		address_to_bitstring_operation && other) noexcept = default;

	inline
	address_to_bitstring_operation(
		const address_to_bitstring_operation & other)
		: result_type_(other.result_type_)
		, original_type_(other.original_type_->copy())
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t nbits() const noexcept { return result_type_.nbits(); }
	inline const jive::base::type & original_type() const noexcept { return *original_type_; }

private:
	jive::bits::type result_type_;
	std::unique_ptr<jive::base::type> original_type_;
};

class bitstring_to_address_operation final : public base::unary_op {
public:
	virtual ~bitstring_to_address_operation() noexcept;

	inline
	bitstring_to_address_operation(
		size_t nbits,
		const jive::base::type * original_type)
		: argument_type_(nbits)
		, original_type_(original_type->copy())
	{
	}

	inline
	bitstring_to_address_operation(
		bitstring_to_address_operation && other) noexcept = default;

	inline
	bitstring_to_address_operation(
		const bitstring_to_address_operation & other)
		: argument_type_(other.argument_type_)
		, original_type_(other.original_type_->copy())
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t nbits() const noexcept { return argument_type_.nbits(); }
	inline const jive::base::type & original_type() const noexcept { return *original_type_; }

private:
	jive::bits::type argument_type_;
	std::unique_ptr<jive::base::type> original_type_;
};

}

typedef jive::operation_node<jive::address_to_bitstring_operation>
	jive_address_to_bitstring_node;

typedef jive::operation_node<jive::bitstring_to_address_operation>
	jive_bitstring_to_address_node;

struct jive_memlayout_mapper;

/* address_to_bitstring node */

extern const jive_node_class JIVE_ADDRESS_TO_BITSTRING_NODE;

jive::output *
jive_address_to_bitstring_create(jive::output * address, size_t nbits,
	const jive::base::type * original_type);

JIVE_EXPORTED_INLINE jive_address_to_bitstring_node *
jive_address_to_bitstring_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_ADDRESS_TO_BITSTRING_NODE))
		return (jive_address_to_bitstring_node *) node;
	else
		return 0;
}

/* bitstring_to_address node */

extern const jive_node_class JIVE_BITSTRING_TO_ADDRESS_NODE;

jive::output *
jive_bitstring_to_address_create(jive::output * bitstring, size_t nbits,
	const jive::base::type * original_type);

JIVE_EXPORTED_INLINE jive_bitstring_to_address_node *
jive_bitstring_to_address_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_BITSTRING_TO_ADDRESS_NODE))
		return (jive_bitstring_to_address_node *) node;
	else
		return 0;
}

/* reductions */

void
jive_load_node_address_transform(jive_load_node * node,
	size_t nbits);

void
jive_store_node_address_transform(jive_store_node * node,
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
