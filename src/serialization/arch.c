#include <jive/serialization/driver.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/typecls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/arch/memory.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>

static void
jive_serialization_stackslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_stackslot_size_class * cls = (const jive_stackslot_size_class *) rescls;
	
	jive_serialize_uint(driver, cls->size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->alignment, os);
}

static bool
jive_serialization_stackslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	
	*rescls = jive_stackslot_size_class_get(size, alignment);
	
	return *rescls != 0;
}

static void
jive_serialization_frameslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_fixed_stackslot_class * cls = (const jive_fixed_stackslot_class *) rescls;
	const jive_stackslot * slot = (const jive_stackslot *) cls->slot;
	
	jive_serialize_uint(driver, cls->base.size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->base.alignment, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_int(driver, slot->offset, os);
}


static bool
jive_serialization_frameslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	int64_t offset;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_int(driver, is, &offset))
		return false;
	
	*rescls = jive_fixed_stackslot_class_get(size, alignment, offset);
	
	return *rescls != 0;
}

static void
jive_serialization_callslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_callslot_class * cls = (const jive_callslot_class *) rescls;
	const jive_callslot * slot = (const jive_callslot *) cls->slot;
	
	jive_serialize_uint(driver, cls->base.size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->base.alignment, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_int(driver, slot->offset, os);
}

static bool
jive_serialization_callslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	int64_t offset;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_int(driver, is, &offset))
		return false;
	
	*rescls = jive_callslot_class_get(size, alignment, offset);
	
	return *rescls != 0;
}

static void
jive_memory_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive_type * type,
	jive_token_ostream * os)
{
	/* no attributes */
}

static bool
jive_memory_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive_type ** type)
{
	JIVE_DECLARE_MEMORY_TYPE(ctl);
	*type = jive_type_copy(ctl, driver->context);
	return true;
}

JIVE_SERIALIZATION_RESCLS_REGISTER(jive_root_register_class, "register");
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_RESOURCE, "stackslot",
	jive_serialization_stackslot_serialize,
	jive_serialization_stackslot_deserialize);
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_FRAMESLOT_RESOURCE, "stack_frameslot",
	jive_serialization_frameslot_serialize,
	jive_serialization_frameslot_deserialize);
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_CALLSLOT_RESOURCE, "stack_callslot",
	jive_serialization_callslot_serialize,
	jive_serialization_callslot_deserialize);
JIVE_SERIALIZATION_TYPECLS_REGISTER(JIVE_MEMORY_TYPE, "memory",
	jive_memory_type_serialize,
	jive_memory_type_deserialize);
