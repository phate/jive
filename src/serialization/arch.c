#include <jive/serialization/driver.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/instrcls-registry.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/typecls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/arch/instruction.h>
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

static void
jive_serialize_immediate(
	jive_serialization_driver * self,
	const jive_immediate * imm,
	jive_token_ostream * os)
{
	/* FIXME: labels! */
	jive_serialize_uint(self, imm->offset, os);
}

static bool
jive_deserialize_immediate(
	jive_serialization_driver * self,
	jive_token_istream * is,
	jive_immediate * imm)
{
	/* FIXME: labels! */
	uint64_t offset;
	if (!jive_deserialize_uint(self, is, &offset))
		return false;
	
	jive_immediate_init(imm, offset, NULL, NULL, NULL);
	return true;
}

static void
jive_instruction_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive_instruction_node_attrs * attrs = (const jive_instruction_node_attrs *) attrs_;
	const jive_serialization_instrcls_registry * reg = driver->instrcls_registry;
	const jive_serialization_instrcls * sercls =
		jive_serialization_instrcls_lookup_by_cls(reg, attrs->icls);
	
	jive_token_ostream_identifier(os, sercls->tag);
	
	size_t n;
	for (n = 0; n < attrs->icls->nimmediates; ++n) {
		if (n != 0)
			jive_token_ostream_char(os, ',');
		const jive_immediate * imm = &attrs->immediates[n];
		jive_serialize_immediate(driver, imm, os);
	}
}

static bool
jive_instruction_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive_output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	const jive_token * token = jive_token_istream_current(is);
	const jive_instruction_class * icls = 0;
	if (token->type == jive_token_identifier) {
		const jive_serialization_instrcls * sercls;
		sercls = jive_serialization_instrcls_lookup_by_tag(
			driver->instrcls_registry, token->v.identifier);
		if (sercls)
			icls = sercls->cls;
	}
	if (!icls) {
		driver->error(driver, "Expected instruction class identifier");
		return false;
	}
	jive_token_istream_advance(is);
	
	jive_immediate * immediates;
	immediates = jive_context_malloc(driver->context,
		icls->nimmediates * sizeof(immediates[0]));
	
	bool parsed_immediates = true;
	size_t index;
	for (index = 0; index < icls->nimmediates; ++ index) {
		if (index != 0) {
			if (!jive_deserialize_char_token(driver, is, ',')) {
				parsed_immediates = false;
				break;
			}
		}
		if (!jive_deserialize_immediate(driver, is, &immediates[index])) {
			parsed_immediates = false;
			break;
		}
	}
	
	if (parsed_immediates) {
		jive_instruction_node_attrs attrs;
		attrs.icls = icls;
		attrs.immediates = immediates;
		
		*node = JIVE_INSTRUCTION_NODE.create(region, &attrs.base,
			noperands, operands);
	}
	
	jive_context_free(driver->context, immediates);
	
	return parsed_immediates;
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
JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_INSTRUCTION_NODE, "instr",
	jive_instruction_serialize,
	jive_instruction_deserialize);
