/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_PRIVATE_H

/* fltbinary operation class inhertiable members */

void
jive_fltbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* fltunary operation class inheritable members */

void
jive_fltunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* fltcomparison operation class inhertiable members */

void
jive_fltcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

#endif
