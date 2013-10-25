/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ITGOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_INTEGRAL_ITGOPERATION_CLASSES_PRIVATE_H

/* itgbinary operation class inhertiable members */

void
jive_itgbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* itgunary operation class inheritable members */

void
jive_itgunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* itgcomparison operation class inhertiable members */

void
jive_itgcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

#endif
