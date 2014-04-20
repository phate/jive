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

extern const jive_unary_operation_class JIVE_MEMBEROF_NODE_;
#define JIVE_MEMBEROF_NODE (JIVE_MEMBEROF_NODE_.base)

namespace jive {
namespace address {

class memberof_operation : public unary_operation {
public:
	inline constexpr
	memberof_operation(
		const jive_record_declaration * record_decl,
		size_t index)
		: record_decl_(record_decl),
		index_(index)
	{
	}

	inline const jive_record_declaration *
	record_decl() const noexcept { return record_decl_; }

	inline size_t
	index() const noexcept { return index_; }

private:
	const jive_record_declaration * record_decl_;
	size_t index_;
};

class containerof_operation : public unary_operation {
public:
	inline constexpr
	containerof_operation(
		const jive_record_declaration * record_decl,
		size_t index)
		: record_decl_(record_decl),
		index_(index)
	{
	}

	inline const jive_record_declaration *
	record_decl() const noexcept { return record_decl_; }

	inline size_t
	index() const noexcept { return index_; }

private:
	const jive_record_declaration * record_decl_;
	size_t index_;
};

class arraysubscript_operation : public operation {
public:
	arraysubscript_operation(const arraysubscript_operation & other);
	arraysubscript_operation(arraysubscript_operation && other) noexcept;
	arraysubscript_operation(const jive_value_type & element_type);

	inline const jive_value_type &
	element_type() const noexcept { return *element_type_; }

private:
	std::unique_ptr<jive_value_type> element_type_;
};

class arrayindex_operation : public jive_node_attrs {
public:
	arrayindex_operation(const arrayindex_operation & other);
	arrayindex_operation(arrayindex_operation && other) noexcept;
	arrayindex_operation(
		const jive_value_type & element_type,
		size_t nbits);

	inline const jive_value_type &
	element_type() const noexcept { return *element_type_; }

	inline const jive_bitstring_type &
	difference_type() const noexcept { return difference_type_; }

private:
	std::unique_ptr<jive_value_type> element_type_;
	jive_bitstring_type difference_type_;
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

jive_output *
jive_memberof(jive_output * address,
	const jive_record_declaration * record_decl, size_t index);

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

extern const jive_unary_operation_class JIVE_CONTAINEROF_NODE_;
#define JIVE_CONTAINEROF_NODE (JIVE_CONTAINEROF_NODE_.base)

typedef jive::operation_node<jive::address::containerof_operation> jive_containerof_node;

jive_output *
jive_containerof(jive_output * address,
	const jive_record_declaration * record_decl, size_t index);

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

extern const jive_binary_operation_class JIVE_ARRAYSUBSCRIPT_NODE_;
#define JIVE_ARRAYSUBSCRIPT_NODE (JIVE_ARRAYSUBSCRIPT_NODE_.base)

typedef jive::operation_node<jive::address::arraysubscript_operation> jive_arraysubscript_node;

jive_output *
jive_arraysubscript(struct jive_output * address, const struct jive_value_type * element_type,
	struct jive_output * index);

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

extern const jive_binary_operation_class JIVE_ARRAYINDEX_NODE_;
#define JIVE_ARRAYINDEX_NODE (JIVE_ARRAYINDEX_NODE_.base)

typedef jive::operation_node<jive::address::arrayindex_operation> jive_arrayindex_node;

jive_output *
jive_arrayindex(struct jive_output * addr1, struct jive_output * addr2,
	const struct jive_value_type * element_type,
	const struct jive_bitstring_type * difference_type);

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

jive_output *
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

jive_output *
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
