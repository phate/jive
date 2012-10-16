/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "rat.h"

#include <jive/internal/instruction_str.h>

extern const jive_cpureg_class jive_RAT_regcls[];

const jive_cpureg jive_RAT_regs [] = {
	[jive_RAT_f0] = {.name = "f0", .regcls = &jive_RAT_regcls[jive_RAT_fizz0], .code = 0, .index = jive_RAT_f0, .class_mask = (1<<jive_RAT_fizz) | (1<<jive_RAT_fizz0)},
	[jive_RAT_f1] = {.name = "f1", .regcls = &jive_RAT_regcls[jive_RAT_fizz1], .code = 1, .index = jive_RAT_f1, .class_mask = (1<<jive_RAT_fizz) | (1<<jive_RAT_fizz1)},
};

const jive_cpureg_class jive_RAT_regcls [] = {
	[jive_RAT_fizz] = {
		.name = "fizz", .nbits = 32,
		.regs = &jive_RAT_regs[jive_RAT_f0], .nregs = 2,
		.index = jive_RAT_fizz, .parent = 0,
		.class_mask = 1<<jive_RAT_fizz
	},
	[jive_RAT_fizz0] = {
		.name = "fizz0", .nbits = 32,
		.regs = &jive_RAT_regs[jive_RAT_f0], .nregs = 1,
		.index = jive_RAT_fizz0, .parent = &jive_RAT_regcls[jive_RAT_fizz],
		.class_mask = (1<<jive_RAT_fizz) | (1<<jive_RAT_fizz0)
	},
	[jive_RAT_fizz1] = {
		.name = "fizz1", .nbits = 32,
		.regs = &jive_RAT_regs[jive_RAT_f1], .nregs = 1,
		.index = jive_RAT_fizz1, .parent = &jive_RAT_regcls[jive_RAT_fizz],
		.class_mask = (1<<jive_RAT_fizz) | (1<<jive_RAT_fizz1)
	}
};

static jive_node *
jive_RAT_do_spill(const jive_machine * machine, jive_value * value, jive_stackslot * where, jive_stackframe * frame);

static jive_value *
jive_RAT_do_restore(const jive_machine * machine, jive_graph * graph, jive_stackslot * where, jive_stackframe * frame);

const jive_machine jive_RAT_machine = {
	.name = "RAT",
	.regcls = jive_RAT_regcls, .nregcls = 3,
	.regs = jive_RAT_regs, .nregs = 2,
	
	.regcls_budget = {
		2, 1, 1,
		0
	},
	
	.spill = jive_RAT_do_spill,
	.restore = jive_RAT_do_restore
};

static const jive_cpureg_class * const fizz_regs[] = {
	&jive_RAT_regcls[jive_RAT_fizz],
	&jive_RAT_regcls[jive_RAT_fizz],
};

static const jive_cpureg_class * const fizz0_regs[] = {
	&jive_RAT_regcls[jive_RAT_fizz0],
};

static const jive_cpureg_class * const fizz1_regs[] = {
	&jive_RAT_regcls[jive_RAT_fizz1],
};

const jive_instruction_class jive_RAT_instructions[] = {
	[jive_RAT_produce] = {
		.name = "produce",
		.encode = 0, .mnemonic = 0,
		.inregs = 0, .outregs = fizz_regs, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_consume] = {
		.name = "consume",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz_regs, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 0, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_combine] = {
		.name = "combine",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz_regs, .outregs = fizz_regs, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_produce_f0] = {
		.name = "produce_f0",
		.encode = 0, .mnemonic = 0,
		.inregs = 0, .outregs = fizz0_regs, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_consume_f0] = {
		.name = "consume_f0",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz0_regs, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 0, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_combine_f0] = {
		.name = "combine_f0",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz_regs, .outregs = fizz0_regs, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_produce_f1] = {
		.name = "produce_f1",
		.encode = 0, .mnemonic = 0,
		.inregs = 0, .outregs = fizz1_regs, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_consume_f1] = {
		.name = "consume_f1",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz1_regs, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 0, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_combine_f1] = {
		.name = "combine_f1",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz_regs, .outregs = fizz1_regs, .flags = jive_instruction_flags_none,
		.ninputs = 2, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_restore] = {
		.name = "restore",
		.encode = 0, .mnemonic = 0,
		.inregs = 0, .outregs = fizz_regs, .flags = jive_instruction_flags_none,
		.ninputs = 0, .noutputs = 1, .nimmediates = 0,
		.code = 0
	},
	[jive_RAT_spill] = {
		.name = "spill",
		.encode = 0, .mnemonic = 0,
		.inregs = fizz_regs, .outregs = 0, .flags = jive_instruction_flags_none,
		.ninputs = 1, .noutputs = 0, .nimmediates = 0,
		.code = 0
	},
};

static jive_node *
jive_RAT_do_spill(const jive_machine * machine, jive_value * value, jive_stackslot * where, jive_stackframe * frame)
{
	jive_node * spill = jive_instruction_create(value->node->graph,
		&jive_RAT_instructions[jive_RAT_spill],
		&value, 0);
	return spill;
}

static jive_value *
jive_RAT_do_restore(const jive_machine * machine, jive_graph * graph, jive_stackslot * where, jive_stackframe * frame)
{
	jive_node * restore = jive_instruction_create(graph,
		&jive_RAT_instructions[jive_RAT_restore],
		0, 0);
	return jive_instruction_output(restore, 0);
}

