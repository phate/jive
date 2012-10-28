/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CODEGEN_H
#define JIVE_ARCH_CODEGEN_H

#include <jive/arch/compilate.h>

struct jive_buffer;
struct jive_graph;
struct jive_seq_graph;

/* FIXME: this is a placeholder function, will be replaced by a more
sophisticated interface later */
void
jive_graph_generate_code(struct jive_graph * graph, struct jive_compilate * buffer);

void
jive_graph_generate_assembler(struct jive_graph * graph, struct jive_buffer * buffer);

void
jive_seq_graph_generate_code(struct jive_seq_graph * seq_graph,
	struct jive_compilate * buffer);

void
jive_seq_graph_generate_assembler(struct jive_seq_graph * seq_graph, struct jive_buffer * buffer);

#endif
