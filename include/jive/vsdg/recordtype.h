#ifndef JIVE_VSDG_RECORD_TYPE_H
#define JIVE_VSDG_RECORD_TYPE_H

#include <jive/vsdg/valuetype.h>

struct jive_value_type;

typedef struct jive_record_declaration jive_record_declaration;

struct jive_record_declaration {
	size_t nelements;
	const jive_value_type ** elements;
};

typedef struct jive_record_type jive_record_type;
typedef struct jive_record_input jive_record_input;
typedef struct jive_record_output jive_record_output;
typedef struct jive_record_gate jive_record_gate;

extern const jive_type_class JIVE_RECORD_TYPE;

struct jive_record_type {
	jive_value_type base;
	const jive_record_declaration * decl;
};

extern const jive_input_class JIVE_RECORD_INPUT;
struct jive_record_input {
	jive_value_input base;
	jive_record_type type;
};

extern const jive_output_class JIVE_RECORD_OUTPUT;
struct jive_record_output {
	jive_value_output base;
	jive_record_type type;
};

extern const jive_gate_class JIVE_RECORD_GATE;
struct jive_record_gate {
	jive_value_gate base;
	jive_record_type type;
};

void
jive_record_type_init(
	jive_record_type * self,
	const jive_record_declaration * decl);

#endif
