/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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

	address_to_bitstring_operation(
		size_t nbits,
		std::unique_ptr<jive::base::type> original_type);

	inline
	address_to_bitstring_operation(
		size_t nbits,
		const jive::base::type & original_type)
		: address_to_bitstring_operation(
			nbits,
			std::unique_ptr<base::type>(original_type.copy()))
	{
	}

	inline
	address_to_bitstring_operation(
		address_to_bitstring_operation && other) noexcept = default;

	inline
	address_to_bitstring_operation(
		const address_to_bitstring_operation & other)
		: nbits_(other.nbits_)
		, result_type_(other.result_type_->copy())
		, original_type_(other.original_type_->copy())
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
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

	inline size_t nbits() const noexcept { return nbits_; }
	inline const jive::base::type & original_type() const noexcept { return *original_type_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	size_t nbits_;
	std::unique_ptr<jive::base::type> result_type_;
	std::unique_ptr<jive::base::type> original_type_;
};

class bitstring_to_address_operation final : public base::unary_op {
public:
	virtual ~bitstring_to_address_operation() noexcept;

	bitstring_to_address_operation(
		size_t nbits,
		std::unique_ptr<jive::base::type> original_type);

	inline
	bitstring_to_address_operation(
		size_t nbits,
		const jive::base::type & original_type)
		: bitstring_to_address_operation(
			nbits,
			std::unique_ptr<base::type>(original_type.copy()))
	{
	}

	inline
	bitstring_to_address_operation(
		bitstring_to_address_operation && other) noexcept = default;

	inline
	bitstring_to_address_operation(
		const bitstring_to_address_operation & other)
		: nbits_(other.nbits_)
		, argument_type_(other.argument_type_->copy())
		, original_type_(other.original_type_->copy())
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
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

	inline size_t nbits() const noexcept { return nbits_; }
	inline const jive::base::type & original_type() const noexcept { return *original_type_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	size_t nbits_;
	std::unique_ptr<jive::base::type> argument_type_;
	std::unique_ptr<jive::base::type> original_type_;
};

class memlayout_mapper;

}

/* address_to_bitstring node */

jive::output *
jive_address_to_bitstring_create(jive::output * address, size_t nbits,
	const jive::base::type * original_type);

/* bitstring_to_address node */

jive::output *
jive_bitstring_to_address_create(jive::output * bitstring, size_t nbits,
	const jive::base::type * original_type);

/* reductions */

void
jive_node_address_transform(jive_node * node, jive::memlayout_mapper * mapper);

void
jive_graph_address_transform(jive_graph * graph, jive::memlayout_mapper * mapper);

#endif
