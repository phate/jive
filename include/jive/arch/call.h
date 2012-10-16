/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CALL_H
#define JIVE_ARCH_CALL_H

#include <jive/common.h>
#include <jive/vsdg/node.h>

struct jive_context;
struct jive_type;

extern const jive_node_class JIVE_CALL_NODE;

typedef struct jive_calling_convention jive_calling_convention;
typedef struct jive_call_node_attrs jive_call_node_attrs;
typedef struct jive_call_node jive_call_node;

/* FIXME: opaque type for now -- to be filled in later */
struct jive_calling_convention;

struct jive_call_node_attrs {
	jive_node_attrs base;
	const jive_calling_convention * calling_convention;
	size_t nreturns;
	jive_type ** return_types;
};

struct jive_call_node {
	jive_node base;
	jive_call_node_attrs attrs;
};

struct jive_node *
jive_call_by_address_node_create(struct jive_region * region,
	struct jive_output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const struct jive_type * const return_types[]);

struct jive_output * const *
jive_call_by_address_create(struct jive_output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const struct jive_type * const return_types[]);

struct jive_node *
jive_call_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const struct jive_type * const return_types[]);

struct jive_output * const *
jive_call_by_bitstring_create(struct jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, struct jive_output * const arguments[],
	size_t nreturns, const struct jive_type * const return_types[]);

JIVE_EXPORTED_INLINE jive_call_node *
jive_call_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_CALL_NODE)
		return (jive_call_node *) node;
	else
		return NULL;
}

#endif
