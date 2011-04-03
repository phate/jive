#ifndef JIVE_VSDG_ANCHORTYPE_H
#define JIVE_VSDG_ANCHORTYPE_H

#include <jive/vsdg/basetype.h>

typedef struct jive_anchor_type jive_anchor_type;
typedef struct jive_anchor_input jive_anchor_input;
typedef struct jive_anchor_output jive_anchor_output;

extern const jive_type_class JIVE_ANCHOR_TYPE;
#define JIVE_DECLARE_ANCHOR_TYPE(name) const jive_anchor_type name##_struct = {{&JIVE_ANCHOR_TYPE}}; const jive_type * name = &name##_struct.base

struct jive_anchor_type {
	jive_type base;
};

extern const jive_input_class JIVE_ANCHOR_INPUT;
struct jive_anchor_input {
	jive_input base;
};

extern const jive_output_class JIVE_ANCHOR_OUTPUT;
struct jive_anchor_output {
	jive_output base;
};

#endif
