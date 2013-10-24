/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_DBLOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_DOUBLE_DBLOPERATION_CLASSES_PRIVATE_H

/* dblbinary operation class inhertiable members */

void
jive_dblbinary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* dblunary operation class inheritable members */

void
jive_dblunary_operation_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

/* dblcomparison operation class inhertiable members */

void
jive_dblcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

#endif
