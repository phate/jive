#ifndef JIVE_VSDG_VALUETYPE_H
#define JIVE_VSDG_VALUETYPE_H

#include <jive/vsdg/basetype.h>
#include <jive/arch/registers.h>

typedef struct jive_value_input jive_value_input;
typedef struct jive_value_output jive_value_output;
typedef struct jive_value_output jive_value;
typedef struct jive_value_gate jive_value_gate;
typedef struct jive_value_resource jive_value_resource;

extern const jive_type_class JIVE_VALUE_TYPE;
extern const jive_type jive_value_type_singleton;

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

extern const jive_resource_class JIVE_VALUE_RESOURCE;
struct jive_value_resource {
	jive_resource base;
	const jive_regcls * regcls;
	const jive_cpureg * cpureg;
};

void
jive_value_resource_set_regcls(jive_value_resource * self, const jive_regcls * regcls);

void
jive_value_resource_set_cpureg(jive_value_resource * self, const jive_cpureg * cpureg);


#endif
