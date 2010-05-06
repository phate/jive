#ifndef JIVE_FIXED_H
#define JIVE_FIXED_H

#include <jive/graph.h>

/* fixed bit-width operations */

extern const jive_node_class JIVE_FIXED;

/* get arithmetic width of fixed-bitwidth operation */
unsigned int
jive_fixed_arithmetic_width(const jive_node * node);

/* sum */

extern const jive_node_class JIVE_FIXEDADD;

jive_node *
jive_fixedadd_rawcreate(jive_value * a, jive_value * b, unsigned int arithmetic_width);

jive_value *
jive_fixedadd_create(jive_value * a, jive_value * b, unsigned int arithmetic_width);

/* multiplication */

extern const jive_node_class JIVE_FIXEDMUL;

jive_node *
jive_fixedmul_rawcreate(jive_value * a, jive_value * b, unsigned int arithmetic_width);

jive_value *
jive_fixedmul_create(jive_value * a, jive_value * b, unsigned int arithmetic_width);

extern const jive_node_class JIVE_FIXEDMULHI;

jive_node *
jive_fixedmulhi_rawcreate(jive_value * a, jive_value * b, unsigned int arithmetic_width);

jive_value *
jive_fixedmulhi_create(jive_value * a, jive_value * b, unsigned int arithmetic_width);

/* negation */

extern const jive_node_class JIVE_FIXEDNEG;

jive_node *
jive_fixedneg_rawcreate(jive_value * input, unsigned int arithmetic_width);

jive_value *
jive_fixedneg_create(jive_value * input, unsigned int arithmetic_width);

/* shift operations */

extern const jive_node_class JIVE_FIXEDSHR;

jive_node *
jive_fixedshr_rawcreate(jive_value * input, unsigned int shift, unsigned int arithmetic_width);

jive_value *
jive_fixedshr_create(jive_value * input, unsigned int shift, unsigned int arithmetic_width);

extern const jive_node_class JIVE_FIXEDSHL;

jive_node *
jive_fixedshl_rawcreate(jive_value * input, unsigned int shift, unsigned int arithmetic_width);

jive_value *
jive_fixedshl_create(jive_value * input, unsigned int shift, unsigned int arithmetic_width);

extern const jive_node_class JIVE_FIXEDASHR;

jive_node *
jive_fixedashr_rawcreate(jive_value * input, unsigned int shift, unsigned int arithmetic_width);

jive_value *
jive_fixedashr_create(jive_value * input, unsigned int shift, unsigned int arithmetic_width);

/* bit operations */

extern const jive_node_class JIVE_FIXEDAND;

jive_node *
jive_fixedand_rawcreate(jive_value * a, jive_value * b);

jive_value *
jive_fixedand_create(jive_value * a, jive_value * b);

extern const jive_node_class JIVE_FIXEDOR;

jive_node *
jive_fixedor_rawcreate(jive_value * a, jive_value * b);

jive_value *
jive_fixedor_create(jive_value * a, jive_value * b);

extern const jive_node_class JIVE_FIXEDXOR;

jive_node *
jive_fixedxor_rawcreate(jive_value * a, jive_value * b);

jive_value *
jive_fixedxor_create(jive_value * a, jive_value * b);

/* constant bitstrings placed in a specific register class */

extern const jive_node_class JIVE_REGCONSTANT;

jive_node *
jive_regconstant_rawcreate(jive_graph * graph, jive_cpureg_class_t regcls, const char * bits);

jive_value *
jive_regconstant_create(jive_graph * graph, jive_cpureg_class_t regcls, const char * bits);

bool
jive_regconstant_match(const jive_node * node, jive_cpureg_class_t regcls, const char * bits);

#endif
