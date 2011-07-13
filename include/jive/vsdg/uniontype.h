#ifndef JIVE_VSDG_UNION_TYPE_H
#define JIVE_VSDG_UNION_TYPE_H

#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/unionlayout.h>

typedef struct jive_union_type jive_union_type;
typedef struct jive_union_input jive_union_input;
typedef struct jive_union_output jive_union_output;
typedef struct jive_union_gate jive_union_gate;

extern const jive_type_class JIVE_UNION_TYPE;

struct jive_union_type {
	jive_value_type base;
	const jive_union_layout * layout;
};

extern const jive_input_class JIVE_UNION_INPUT;
struct jive_union_input {
	jive_value_input base;
	jive_union_type type;
};

extern const jive_output_class JIVE_UNION_OUTPUT;
struct jive_union_output {
	jive_value_output base;
	jive_union_type type;
};

extern const jive_gate_class JIVE_UNION_GATE;
struct jive_union_gate {
	jive_value_gate base;
	jive_union_type type;
};

void
jive_union_type_init(
	jive_union_type * self,
	const jive_union_layout * layout);

#endif
