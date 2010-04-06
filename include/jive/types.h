#ifndef JIVE_TYPES_H
#define JIVE_TYPES_H

#include <inttypes.h>
#include <stdlib.h>

/* forward-definitions of various public data types */

/* FIXME: decide which really must go here */

typedef enum {jive_endian_little, jive_endian_big} jive_endian;

typedef struct jive_node jive_node;
typedef struct jive_value jive_value;
typedef struct jive_operand jive_operand;

typedef const struct jive_cpureg * jive_cpureg_t;
typedef const struct jive_cpureg_class * jive_cpureg_class_t;
typedef uint32_t jive_cpureg_classmask_t;

/** \brief A value transferred across a branch point */
typedef struct _jive_passthrough jive_passthrough;

/** \brief Machine description */
typedef struct jive_machine jive_machine;
typedef struct jive_instruction_class jive_instruction_class;

/** \brief Horizontal cut through graph */
typedef struct jive_graphcut jive_graphcut;

/** \brief Stackframe */
typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackslot jive_stackslot;

/** \brief Frame for callable procedure or function */
typedef struct _jive_subroutine jive_subroutine;

typedef enum {
	jive_regalloc_regstate_none = 0,
	jive_regalloc_regstate_newly_active = 1,
	jive_regalloc_regstate_readers_active = 2,
	jive_regalloc_regstate_readers_done = 3,
	jive_regalloc_regstate_shaped = 4,
	jive_regalloc_regstate_done = 5,
	jive_regalloc_regstate_temp_marker = 128
} jive_regalloc_regstate;

typedef enum {
	jive_regalloc_inststate_none = 0,
	jive_regalloc_inststate_available = 1,
	jive_regalloc_inststate_selected = 2,
	jive_regalloc_inststate_available_next = 3,
	jive_regalloc_inststate_shaped = 4,
	jive_regalloc_inststate_done = 5,
	jive_regalloc_inststate_temp_marker = 128
} jive_regalloc_inststate;

#endif
