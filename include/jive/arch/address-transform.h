/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESS_TRANSFORM_H
#define JIVE_ARCH_ADDRESS_TRANSFORM_H

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>

struct jive_memlayout_mapper;

/* address_to_bitstring node */

extern const jive_unary_operation_class JIVE_ADDRESS_TO_BITSTRING_NODE_;
#define JIVE_ADDRESS_TO_BITSTRING_NODE (JIVE_ADDRESS_TO_BITSTRING_NODE_.base)

typedef struct jive_address_to_bitstring_node jive_address_to_bitstring_node;
typedef struct jive_address_to_bitstring_node_attrs jive_address_to_bitstring_node_attrs;

struct jive_address_to_bitstring_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	jive_type * original_type;
};

struct jive_address_to_bitstring_node {
	jive_node base;
	jive_address_to_bitstring_node_attrs attrs;
};

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

typedef struct jive_bitstring_to_address_node jive_bitstring_to_address_node;
typedef struct jive_bitstring_to_address_node_attrs jive_bitstring_to_address_node_attrs;

struct jive_bitstring_to_address_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	jive_type * original_type;
};

struct jive_bitstring_to_address_node {
	jive_node base;
	jive_bitstring_to_address_node_attrs attrs;
};

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

struct jive_apply_node;
struct jive_lambda_node;
struct jive_load_node;
struct jive_store_node;

void
jive_load_node_address_transform(struct jive_load_node * node,
	size_t nbits);

void
jive_store_node_address_transform(struct jive_store_node * node,
	size_t nbits);

struct jive_label_to_address_node;

void
jive_label_to_address_node_address_transform(struct jive_label_to_address_node * node,
	size_t nbits);

struct jive_call_node;

void
jive_call_node_address_transform(struct jive_call_node * node,
	size_t nbits);

void
jive_lambda_node_address_transform(const struct jive_lambda_node * node, size_t nbits);

void
jive_apply_node_address_transform(const struct jive_apply_node * node, size_t nbits);

struct jive_memberof_node;
struct jive_containerof_node;
struct jive_arraysubscript_node;
struct jive_arrayindex_node;

void
jive_memberof_node_address_transform(struct jive_memberof_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_containerof_node_address_transform(struct jive_containerof_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_arraysubscript_node_address_transform(struct jive_arraysubscript_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_arrayindex_node_address_transform(struct jive_arrayindex_node * node,
	struct jive_memlayout_mapper * mapper);

void
jive_graph_address_transform(jive_graph * graph,
	struct jive_memlayout_mapper * mapper);

#endif
