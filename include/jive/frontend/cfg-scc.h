/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_SCC_H
#define JIVE_FRONTEND_CFG_SCC_H

#include <unordered_set>
#include <vector>

namespace jive {
namespace frontend {

class cfg;
class cfg_node;

std::vector<std::unordered_set<jive::frontend::cfg_node*>>
jive_cfg_find_sccs(const jive::frontend::cfg & self);

}
}

#endif
