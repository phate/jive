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
	cfg.exit->divert_predecessors(bb1);
	bb1->add_nottaken_successor(bb2);
	bb2->add_nottaken_successor(bb3);
	bb3->add_nottaken_successor(bb1);

	/* second scc */
	bb4->add_nottaken_successor(bb2);
	bb4->add_taken_successor(bb6);
	bb6->add_nottaken_successor(bb4);
	bb6->add_taken_successor(bb5);

	/* third scc */
	bb5->add_nottaken_successor(bb3);
	bb5->add_taken_successor(bb7);
	bb7->add_nottaken_successor(bb5);
	bb7->add_taken_successor(bb8);

	/* fourth scc */
	bb8->add_taken_successor(bb8);
	bb8->add_nottaken_successor(cfg.exit);
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
