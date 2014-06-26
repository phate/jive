/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESS_H
#define JIVE_ARCH_ADDRESS_H

#include <memory>

#include <jive/arch/addresstype.h>
#include <jive/common.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdtype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

struct jive_label;

/* "memberof" operator: given an address that is the start of a record
in memory, compute address of specified member of record */

extern const jive_node_class JIVE_MEMBEROF_NODE;

namespace jive {
namespace address {

class memberof_operation : public base::unary_op {
public:
	virtual ~memberof_operation() noexcept;

	inline constexpr
	memberof_operation(
		const jive::rcd::declaration * record_decl,
		size_t index)
		: record_decl_(record_decl),
		index_(index)
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

	inline const jive::rcd::declaration *
	record_decl() const noexcept { return record_decl_; }

	inline size_t
	index() const noexcept { return index_; }

private:
	const jive::rcd::declaration * record_decl_;
	size_t index_;
};

class containerof_operation : public base::unary_op {
public:
	virtual ~containerof_operation() noexcept;

	inline constexpr
	containerof_operation(
		const jive::rcd::declaration * record_decl,
		size_t index)
		: record_decl_(record_decl),
		index_(index)
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

	inline const jive::rcd::declaration *
	record_decl() const noexcept { return record_decl_; }

	inline size_t
	index() const noexcept { return index_; }

private:
	const jive::rcd::declaration * record_decl_;
	size_t index_;
};

class arraysubscript_operation : public operation {
public:
	virtual ~arraysubscript_operation() noexcept;

	arraysubscript_operation(const arraysubscript_operation & other);
	arraysubscript_operation(arraysubscript_operation && other) noexcept;
	arraysubscript_operation(const jive::value::type & element_type);

	inline const jive::value::type &
	element_type() const noexcept { return *element_type_; }

private:
	std::unique_ptr<jive::value::type> element_type_;
};

class arrayindex_operation : public operation {
public:
	virtual ~arrayindex_operation() noexcept;

	arrayindex_operation(const arrayindex_operation & other);
	arrayindex_operation(arrayindex_operation && other) noexcept;
	arrayindex_operation(
		const jive::value::type & element_type,
		size_t nbits);

	inline const jive::value::type &
	element_type() const noexcept { return *element_type_; }

	inline const jive::bits::type &
	difference_type() const noexcept { return difference_type_; }

private:
	std::unique_ptr<jive::value::type> element_type_;
	jive::bits::type difference_type_;
};

class label_to_address_operation : public nullary_operation {
public:
	inline constexpr
	label_to_address_operation(const jive_label * label) noexcept
		: label_(label)
	{
	}

	const jive_label *
	label() const noexcept { return label_; }

private:
	const struct jive_label * label_;
};

class label_to_bitstring_operation : public nullary_operation {
public:
	inline constexpr
	label_to_bitstring_operation(
		const jive_label * label,
		size_t nbits) noexcept
		: label_(label), nbits_(nbits)
	{
	}

	const jive_label *
	label() const noexcept { return label_; }

	size_t
	nbits() const noexcept { return nbits_; }

private:
	const jive_label * label_;
	size_t nbits_;
};

}
}

typedef jive::operation_node<jive::address::memberof_operation> jive_memberof_node;

jive::output *
jive_memberof(jive::output * address, const jive::rcd::declaration * record_decl, size_t index);

JIVE_EXPORTED_INLINE jive_memberof_node *
jive_memberof_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_MEMBEROF_NODE))
		return (jive_memberof_node *) node;
	else
		return NULL;
}

/* "containerof" operator: given an address that is the start of a record
member in memory, compute address of containing record */

extern const jive_node_class JIVE_CONTAINEROF_NODE;

typedef jive::operation_node<jive::address::containerof_operation> jive_containerof_node;

jive::output *
jive_containerof(jive::output * address, const jive::rcd::declaration * record_decl, size_t index);

JIVE_EXPORTED_INLINE jive_containerof_node *
jive_containerof_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_CONTAINEROF_NODE))
		return (jive_containerof_node *) node;
	else
		return NULL;
}

/* "arraysubscript" operator: given an address that points to an element of
an array, compute address of element offset by specified distance */

extern const jive_node_class JIVE_ARRAYSUBSCRIPT_NODE;

typedef jive::operation_node<jive::address::arraysubscript_operation> jive_arraysubscript_node;

jive::output *
jive_arraysubscript(jive::output * address, const jive::value::type * element_type,
	jive::output * index);

JIVE_EXPORTED_INLINE jive_arraysubscript_node *
jive_arraysubscript_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ARRAYSUBSCRIPT_NODE))
		return (jive_arraysubscript_node *) node;
	else
		return NULL;
}

/* "arrayindex" operator: given two addresses that each point to an
element of an array and the array element type, compute the
difference of their indices */

extern const jive_node_class JIVE_ARRAYINDEX_NODE;

typedef jive::operation_node<jive::address::arrayindex_operation> jive_arrayindex_node;

jive::output *
jive_arrayindex(jive::output * addr1, jive::output * addr2,
	const jive::value::type * element_type,
	const jive::bits::type * difference_type);

JIVE_EXPORTED_INLINE jive_arrayindex_node *
jive_arrayindex_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ARRAYINDEX_NODE))
		return (jive_arrayindex_node *) node;
	else
		return NULL;
}

/* label_to_address node */

extern const jive_node_class JIVE_LABEL_TO_ADDRESS_NODE;

typedef jive::operation_node<jive::address::label_to_address_operation> jive_label_to_address_node;

jive::output *
jive_label_to_address_create(struct jive_graph * graph, const struct jive_label * label);

JIVE_EXPORTED_INLINE jive_label_to_address_node *
jive_label_to_address_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_LABEL_TO_ADDRESS_NODE))
		return (jive_label_to_address_node *) node;
	else
		return 0;
}

/* label_to_bitstring node */

extern const jive_node_class JIVE_LABEL_TO_BITSTRING_NODE;

typedef jive::operation_node<jive::address::label_to_bitstring_operation>
	jive_label_to_bitstring_node;

jive::output *
jive_label_to_bitstring_create(
	struct jive_graph * graph, const struct jive_label * label, size_t nbits);

JIVE_EXPORTED_INLINE jive_label_to_bitstring_node *
jive_label_to_bitstring_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_LABEL_TO_BITSTRING_NODE))
		return (jive_label_to_bitstring_node *) node;
	else
		return 0;
}

#endif
