#ifndef JIVE_SERIALIZATION_RESCLS_REGISTRY_H
#define JIVE_SERIALIZATION_RESCLS_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>

struct jive_resource_class;
struct jive_resource_class_class;
struct jive_output;
struct jive_region;
struct jive_serialization_driver;
struct jive_token_ostream;
struct jive_token_istream;

typedef struct jive_serialization_rescls jive_serialization_rescls;

typedef void (*jive_rescls_serialize_function_t)(
	const jive_serialization_rescls * self,
	struct jive_serialization_driver * driver,
	const struct jive_resource_class * attrs,
	struct jive_token_ostream * os);

typedef bool (*jive_rescls_deserialize_function_t)(
	const jive_serialization_rescls * self,
	struct jive_serialization_driver * driver,
	struct jive_token_istream * is,
	const struct jive_resource_class ** rescls);

struct jive_serialization_rescls {
	const char * tag;
	const void * cls;
	jive_rescls_serialize_function_t serialize;
	jive_rescls_deserialize_function_t deserialize;
	
	bool is_meta_class;
	
	struct {
		jive_serialization_rescls * prev, * next;
	} tag_chain;
	struct {
		jive_serialization_rescls * prev, * next;
	} cls_chain;
	
};

typedef struct jive_serialization_rescls_registry jive_serialization_rescls_registry;

const jive_serialization_rescls_registry *
jive_serialization_rescls_registry_get(void);

JIVE_EXPORTED_INLINE void
jive_serialization_rescls_registry_put(
	const jive_serialization_rescls_registry * reg)
{
}

const jive_serialization_rescls *
jive_serialization_rescls_lookup_by_cls(
	const jive_serialization_rescls_registry * self,
	const struct jive_resource_class * cls);

const jive_serialization_rescls *
jive_serialization_rescls_lookup_by_tag(
	const jive_serialization_rescls_registry * self,
	const char * tag);

void
jive_serialization_rescls_register(
	const void * entity,
	const char tag[],
	bool is_meta_class,
	jive_rescls_serialize_function_t serialize,
	jive_rescls_deserialize_function_t deserialize);

void
jive_serialization_rescls_serialize_default(
	const jive_serialization_rescls * self,
	struct jive_serialization_driver * driver,
	const struct jive_resource_class * rescls,
	struct jive_token_ostream * os);

bool
jive_serialization_rescls_deserialize_default(
	const jive_serialization_rescls * self,
	struct jive_serialization_driver * driver,
	struct jive_token_istream * is,
	const struct jive_resource_class ** rescls);

#define JIVE_SERIALIZATION_RESCLS_REGISTER(rescls, tag) \
	static void __attribute__((constructor)) register_##rescls(void)\
	{ \
		jive_serialization_rescls_register(&rescls, tag, false, \
			jive_serialization_rescls_serialize_default, \
			jive_serialization_rescls_deserialize_default); \
	} \


#define JIVE_SERIALIZATION_META_RESCLS_REGISTER(resclscls, tag, serialize, deserialize) \
	static void __attribute__((constructor)) register_##resclscls(void)\
	{ \
		jive_serialization_rescls_register(&resclscls, tag, true, serialize, deserialize); \
	} \

#endif
