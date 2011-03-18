#ifndef JIVE_VSDG_VALUETYPE_H
#define JIVE_VSDG_VALUETYPE_H

#include <jive/vsdg/basetype.h>
#include <jive/arch/registers.h>
#include <jive/util/hash.h>

typedef struct jive_value_type jive_value_type;
typedef struct jive_value_input jive_value_input;
typedef struct jive_value_output jive_value_output;
typedef struct jive_value_output jive_value;
typedef struct jive_value_gate jive_value_gate;
typedef struct jive_value_resource jive_value_resource;

typedef struct jive_value_allowed_register jive_value_allowed_register;
struct jive_value_allowed_register {
	const struct jive_cpureg * reg;
	struct {
		jive_value_allowed_register * prev;
		jive_value_allowed_register * next;
	} chain;
};
JIVE_DECLARE_HASH_TYPE(jive_allowed_registers_hash, jive_value_allowed_register, const struct jive_cpureg *, reg, chain);
typedef struct jive_allowed_registers_hash jive_allowed_registers_hash;

extern const jive_type_class JIVE_VALUE_TYPE;
#define JIVE_DECLARE_VALUE_TYPE(name) const jive_value_type name##_struct = {{&JIVE_VALUE_TYPE}}; const jive_type * name = &name##_struct.base

struct jive_value_type {
	jive_type base;
};

extern const jive_input_class JIVE_VALUE_INPUT;
struct jive_value_input {
	jive_input base;
	const jive_regcls * required_regcls;
};

extern const jive_output_class JIVE_VALUE_OUTPUT;
struct jive_value_output {
	jive_output base;
	const jive_regcls * required_regcls;
};

extern const jive_gate_class JIVE_VALUE_GATE;
struct jive_value_gate {
	jive_gate base;
	const jive_regcls * required_regcls;
};

#endif
