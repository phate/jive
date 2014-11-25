/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CODEGEN_H
#define JIVE_ARCH_CODEGEN_H

#include <jive/arch/compilate.h>

struct jive_buffer;
struct jive_graph;
struct jive_label_name_mapper;
struct jive_label_symbol_mapper;
struct jive_seq_graph;

/* compile given graph; note that any previous contents of the target buffer will
 * be discarded */
void
jive_graph_generate_code(
	struct jive_graph * graph,
	struct jive_label_symbol_mapper * sym_mapper,
	struct jive_compilate * buffer);

void
jive_graph_generate_assembler(
	struct jive_graph * graph,
	struct jive_label_name_mapper * mapper,
	struct jive_buffer * buffer);

/* compile given graph; note that any previous contents of the target buffer will
 * be discarded */
void
jive_seq_graph_generate_code(
	struct jive_seq_graph * seq_graph,
	struct jive_label_symbol_mapper * sym_mapper,
	struct jive_compilate * buffer);

void
jive_seq_graph_generate_assembler(
	struct jive_seq_graph * seq_graph,
	struct jive_label_name_mapper * mapper,
	struct jive_buffer * buffer);

#endif
