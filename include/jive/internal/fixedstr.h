#ifndef JIVE_INTERNAL_FIXEDSTR_H
#define JIVE_INTERNAL_FIXEDSTR_H

#include <jive/fixed.h>
#include <jive/bitstring.h>

typedef struct jive_fixed_base jive_fixed_base;
typedef struct jive_fixed_binaryop jive_fixed_binaryop;
typedef struct jive_fixed_unaryop jive_fixed_unaryop;
typedef struct jive_fixed_shift jive_fixed_shift;

struct jive_fixed_base {
	jive_node base;
	jive_value_bits output;
	unsigned short arithmetic_width;
};

struct jive_fixed_binaryop {
	jive_node base;
	jive_value_bits output;
	unsigned short arithmetic_width;
	jive_operand_bits * inputs;
};

struct jive_fixed_unaryop {
	jive_node base;
	jive_value_bits output;
	unsigned short arithmetic_width;
	jive_operand_bits * inputs;
};

struct jive_fixed_shift {
	jive_node base;
	jive_value_bits output;
	unsigned short arithmetic_width;
	jive_operand_bits * inputs;
	unsigned short shift;
};

struct jive_regconstant {
	jive_node base;
	jive_value_bits output;
	char * bits;
};

#endif
