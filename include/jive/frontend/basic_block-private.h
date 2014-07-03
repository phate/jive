/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_BASIC_BLOCK_PRIVATE_H
#define JIVE_FRONTEND_BASIC_BLOCK_PRIVATE_H

struct jive_basic_block;
struct jive_cfg;
struct jive_cfg_node;

void
jive_basic_block_init_(struct jive_basic_block * self, struct jive_cfg * cfg);

void
jive_cfg_node_get_label_(const struct jive_cfg_node * self, struct jive_buffer * buffer);

struct jive_cfg_node *
jive_cfg_node_create_(struct jive_cfg * cfg);

#endif
