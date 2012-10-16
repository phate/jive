/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RAT
#define JIVE_RAT

#include <jive/machine.h>

/* a wicked pseudo-architecture with strange numbers of
registers and even stranger instruction constraints;
used for exercising the register allocator */

extern const jive_machine jive_RAT_machine;
extern const jive_instruction_class jive_RAT_instructions[];

typedef enum {
	jive_RAT_fizz = 0,
	jive_RAT_fizz0 = 1,
	jive_RAT_fizz1 = 2,
} jive_RAT_regcls_index;

typedef enum {
	jive_RAT_f0 = 0,
	jive_RAT_f1 = 1
} jive_i386_reg_index;

typedef enum {
	jive_RAT_produce = 0,
	jive_RAT_consume = 1,
	jive_RAT_combine = 2,
	jive_RAT_produce_f0 = 3,
	jive_RAT_consume_f0 = 4,
	jive_RAT_combine_f0 = 5,
	jive_RAT_produce_f1 = 6,
	jive_RAT_consume_f1 = 7,
	jive_RAT_combine_f1 = 8,
	jive_RAT_restore = 9,
	jive_RAT_spill = 10,
} jive_RAT_instruction_index;

#endif
