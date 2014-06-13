/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_PRIVATE_H

#include <jive/vsdg/node.h>

/* fltcomparison operation class inhertiable members */

void
jive_fltcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive::output * const operands[],
	jive_context * context);

#endif
