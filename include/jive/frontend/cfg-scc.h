/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_SCC_H
#define JIVE_FRONTEND_CFG_SCC_H

#include <unordered_set>
#include <vector>

class jive_cfg;
class jive_cfg_node;

std::vector<std::unordered_set<jive_cfg_node*>>
jive_cfg_find_sccs(jive_cfg * self);

#endif
