/*
 * Copyright 2013 2014 Nico Reissmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg-scc.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

#include <assert.h>

static jive::frontend::basic_block * bb1;
static jive::frontend::basic_block * bb2;
static jive::frontend::basic_block * bb3;
static jive::frontend::basic_block * bb4;
static jive::frontend::basic_block * bb5;
static jive::frontend::basic_block * bb6;
static jive::frontend::basic_block * bb7;
static jive::frontend::basic_block * bb8;

static void
setup_cfg(jive::frontend::cfg & cfg)
{
	bb1 = new jive::frontend::basic_block(cfg);
	bb2 = new jive::frontend::basic_block(cfg);
	bb3 = new jive::frontend::basic_block(cfg);
	bb4 = new jive::frontend::basic_block(cfg);
	bb5 = new jive::frontend::basic_block(cfg);
	bb6 = new jive::frontend::basic_block(cfg);
	bb7 = new jive::frontend::basic_block(cfg);
	bb8 = new jive::frontend::basic_block(cfg);

	/* first scc */
	cfg.exit->divert_inedges(bb1);
	bb1->add_outedge(bb2, 0);
	bb2->add_outedge(bb3, 0);
	bb3->add_outedge(bb1, 0);

	/* second scc */
	bb4->add_outedge(bb2, 0);
	bb4->add_outedge(bb6, 1);
	bb6->add_outedge(bb4, 0);
	bb6->add_outedge(bb5, 1);

	/* third scc */
	bb5->add_outedge(bb3, 0);
	bb5->add_outedge(bb7, 1);
	bb7->add_outedge(bb5, 0);
	bb7->add_outedge(bb8, 1);

	/* fourth scc */
	bb8->add_outedge(bb8, 1);
	bb8->add_outedge(cfg.exit, 0);
}

static void
check_sccs(std::vector<std::unordered_set<jive::frontend::cfg_node*>> & sccs)
{
	/* we have 6 sccs, since enter and exit form their own scc */
	assert(sccs.size() == 6);

	for (auto scc : sccs) {
		if (scc.size() == 3) {
			assert(scc.find(bb1) != scc.end());
			assert(scc.find(bb2) != scc.end());
			assert(scc.find(bb3) != scc.end());
		}

		if (scc.size() == 2) {
			if (scc.find(bb4) != scc.end()) {
				assert(scc.find(bb6) != scc.end());
			} else {
				assert(scc.find(bb5) != scc.end());
				assert(scc.find(bb7) != scc.end());
			}
		}
	}
}

static int
test_main(void)
{
	jive::frontend::cfg cfg;

	setup_cfg(cfg);

//	jive_cfg_view(cfg);

	std::vector<std::unordered_set<jive::frontend::cfg_node*>> sccs = jive_cfg_find_sccs(cfg);
	check_sccs(sccs);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("frontend/test-cfg-scc", test_main);
