/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_BASIC_BLOCK_PRIVATE_H
#define JIVE_FRONTEND_BASIC_BLOCK_PRIVATE_H

struct jive_basic_block;
struct jive_cfg;

void
jive_basic_block_init_(struct jive_basic_block * self, struct jive_cfg * cfg);

#endif
