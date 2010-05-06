#ifndef JIVE_SUBROUTINE_H
#define JIVE_SUBROUTINE_H

#include <jive/types.h>

struct jive_graph;

/** \brief Get entry node of subroutine */
jive_node *
jive_subroutine_enter(jive_subroutine * sub);

/** \brief Get exit node of subroutine */
jive_node *
jive_subroutine_leave(jive_subroutine * sub);

/** \brief Get stackframe associated with subroutine */
jive_stackframe *
jive_subroutine_stackframe(jive_subroutine * sub);

/** \brief Get parameter value */
jive_value *
jive_subroutine_parameter(jive_subroutine * sub, size_t index);

/** \brief Set return value of subroutine */
void
jive_subroutine_return_value(jive_subroutine * sub, jive_value * value);

jive_stackslot *
jive_stackframe_allocate_slot(jive_stackframe * frame, jive_cpureg_class_t regcls);

/* FIXME: it is probably preferrable to finalize the whole
subroutine frame instead */
void
jive_stackframe_finalize(jive_stackframe * frame);

jive_stackframe *
jive_default_stackframe_create(struct jive_graph * graph);


/**
	\brief Function argument type
*/
typedef enum jive_argument_type {
	jive_argument_void = 0, /* only legal as return type */
	jive_argument_pointer = 1,
	jive_argument_int = 2,
	jive_argument_long = 3
} jive_argument_type;

#endif
