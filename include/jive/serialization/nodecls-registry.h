/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_NODECLS_REGISTRY_H
#define JIVE_SERIALIZATION_NODECLS_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>

struct jive_node;
struct jive_node_attrs;
struct jive_node_class;
struct jive_output;
struct jive_region;
struct jive_serialization_driver;
struct jive_token_ostream;
struct jive_token_istream;

typedef struct jive_serialization_nodecls jive_serialization_nodecls;

typedef void (*jive_nodecls_serialize_function_t)(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const struct jive_node_attrs * attrs,
	struct jive_token_ostream * os);

typedef bool (*jive_nodecls_deserialize_function_t)(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	struct jive_region * region,
	size_t noperands, struct jive_output * const operands[],
	struct jive_token_istream * is,
	struct jive_node ** node);

struct jive_serialization_nodecls {
	const char * tag;
	const struct jive_node_class * cls;
	jive_nodecls_serialize_function_t serialize;
	jive_nodecls_deserialize_function_t deserialize;
	
	struct {
		jive_serialization_nodecls * prev, * next;
	} tag_chain;
	struct {
		jive_serialization_nodecls * prev, * next;
	} cls_chain;
	
};

typedef struct jive_serialization_nodecls_registry jive_serialization_nodecls_registry;

const jive_serialization_nodecls_registry *
jive_serialization_nodecls_registry_get(void);

JIVE_EXPORTED_INLINE void
jive_serialization_nodecls_registry_put(
	const jive_serialization_nodecls_registry * reg)
{
}

const jive_serialization_nodecls *
jive_serialization_nodecls_lookup_by_cls(
	const jive_serialization_nodecls_registry * self,
	const struct jive_node_class * cls);

const jive_serialization_nodecls *
jive_serialization_nodecls_lookup_by_tag(
	const jive_serialization_nodecls_registry * self,
	const char * tag);

void
jive_serialization_nodecls_register(
	const struct jive_node_class * nodecls,
	const char tag[],
	jive_nodecls_serialize_function_t serialize,
	jive_nodecls_deserialize_function_t deserialize);

void
jive_serialization_nodecls_serialize_default(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const struct jive_node_attrs * attrs,
	struct jive_token_ostream * os);

bool
jive_serialization_nodecls_deserialize_default(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	struct jive_region * region,
	size_t noperands, struct jive_output * const operands[],
	struct jive_token_istream * is,
	struct jive_node ** node);

#define JIVE_SERIALIZATION_NODECLS_REGISTER(nodecls, tag, serialize, deserialize) \
	static void __attribute__((constructor)) register_##nodecls(void)\
	{ \
		jive_serialization_nodecls_register(&nodecls, tag, serialize, deserialize); \
	} \

#define JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(nodecls, tag) \
	static void __attribute__((constructor)) register_##nodecls(void)\
	{ \
		jive_serialization_nodecls_register(&nodecls, tag, \
			jive_serialization_nodecls_serialize_default, \
			jive_serialization_nodecls_deserialize_default); \
	} \

#endif
