/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_SYMBOLIC_EXPRESSION_H
#define JIVE_TYPES_BITSTRING_SYMBOLIC_EXPRESSION_H

struct jive_context;
struct jive_output;

char *
jive_bitstring_symbolic_expression(struct jive_context * context, struct jive_output * output);

#endif
