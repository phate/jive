/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_SERIALIZATION_INSTRCLS_REGISTRY_H
#define JIVE_SERIALIZATION_INSTRCLS_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>

struct jive_instruction_class;
struct jive_serialization_driver;
struct jive_token_ostream;
struct jive_token_istream;

typedef struct jive_serialization_instrcls jive_serialization_instrcls;

struct jive_serialization_instrcls {
	const char * tag;
	const struct jive_instruction_class * cls;
	
	struct {
		jive_serialization_instrcls * prev, * next;
	} tag_chain;
	struct {
		jive_serialization_instrcls * prev, * next;
	} cls_chain;
	
};

typedef struct jive_serialization_instrcls_registry jive_serialization_instrcls_registry;

const jive_serialization_instrcls_registry *
jive_serialization_instrcls_registry_get(void);

JIVE_EXPORTED_INLINE void
jive_serialization_instrcls_registry_put(
	const jive_serialization_instrcls_registry * reg)
{
}

const jive_serialization_instrcls *
jive_serialization_instrcls_lookup_by_cls(
	const jive_serialization_instrcls_registry * self,
	const struct jive_instruction_class * cls);

const jive_serialization_instrcls *
jive_serialization_instrcls_lookup_by_tag(
	const jive_serialization_instrcls_registry * self,
	const char * tag);

void
jive_serialization_instrcls_register(
	const struct jive_instruction_class * instrcls,
	const char tag[]);

void
jive_serialization_instrset_register(
	const struct jive_instruction_class * const * instrclss,
	size_t ninstrclss,
	const char prefix[]);

#define JIVE_SERIALIZATION_INSTRCLS_REGISTER(instrcls, tag) \
	static void __attribute__((constructor)) register_##instrcls(void)\
	{ \
		jive_serialization_instrcls_register(&instrcls, tag); \
	} \

#define JIVE_SERIALIZATION_INSTRSET_REGISTER(instrset, count, prefix) \
\
	static void __attribute__((constructor)) register_##instrset(void)\
	{ \
		jive_serialization_instrset_register(instrset, count, prefix); \
	} \

#endif
