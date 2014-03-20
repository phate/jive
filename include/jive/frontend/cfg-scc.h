/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_SCC_H
#define JIVE_FRONTEND_CFG_SCC_H

#include <jive/util/set.h>

struct jive_cfg;
struct jive_cfg_node;

JIVE_DECLARE_SET_TYPE(jive_cfg_scc, struct jive_cfg_node);
JIVE_DEFINE_SET_TYPE(jive_cfg_scc, struct jive_cfg_node);

JIVE_DECLARE_SET_TYPE(jive_cfg_scc_set,  struct jive_cfg_scc);
JIVE_DEFINE_SET_TYPE(jive_cfg_scc_set, struct jive_cfg_scc);

typedef struct jive_cfg_scc jive_cfg_scc;
typedef struct jive_cfg_scc_set jive_cfg_scc_set;

struct jive_cfg_scc_set *
jive_cfg_scc_set_create(struct jive_context * context);

void
jive_cfg_scc_set_destroy(struct jive_cfg_scc_set * self);

void
jive_cfg_find_sccs(struct jive_cfg * self, struct jive_cfg_scc_set * scc_set);

#endif
