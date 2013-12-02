/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_PRIVATE_H

/* bitbinary operation class inhertiable members */

void
jive_bitbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* bitunary operation class inheritable members */

void
jive_bitunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* bitcomparison operation class inhertiable members */

void
jive_bitcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

#endif
