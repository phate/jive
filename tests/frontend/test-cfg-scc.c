/*
 * Copyright 2013 2014 Nico Reissmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg-scc.h>
#include <jive/frontend/tac/variable.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

#include <assert.h>

static jive_basic_block * bb1;
static jive_basic_block * bb2;
static jive_basic_block * bb3;
static jive_basic_block * bb4;
static jive_basic_block * bb5;
static jive_basic_block * bb6;
static jive_basic_block * bb7;
static jive_basic_block * bb8;

static void
setup_cfg(jive_cfg * cfg)
{
	bb1 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb2 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb3 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb4 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb5 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb6 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb7 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));
	bb8 = dynamic_cast<jive_basic_block*>(jive_basic_block_create(cfg));

	jive_variable_code_create(bb1, "1");
	jive_variable_code_create(bb2, "2");
	jive_variable_code_create(bb3, "3");
	jive_variable_code_create(bb4, "4");
	jive_variable_code_create(bb5, "5");
	jive_variable_code_create(bb6, "6");
	jive_variable_code_create(bb7, "7");
	jive_variable_code_create(bb8, "8");

	/* first scc */
	cfg->exit->divert_predecessors(bb1);
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
	bb8->add_nottaken_successor(cfg->exit);
}

static void
check_sccs(jive_cfg_scc_set * scc_set)
{
	/* we have 6 sccs, since enter and exit form their own scc */
	assert(jive_cfg_scc_set_size(scc_set) == 6);

	struct jive_cfg_scc_set_iterator i;
	JIVE_SET_ITERATE(jive_cfg_scc_set, *scc_set, i) {
		if (jive_cfg_scc_size(i.entry->item) == 3) {
			assert(jive_cfg_scc_contains(i.entry->item, bb1));
			assert(jive_cfg_scc_contains(i.entry->item, bb2));
			assert(jive_cfg_scc_contains(i.entry->item, bb3));
		}

		if (jive_cfg_scc_size(i.entry->item) == 2) {
			bool contained = jive_cfg_scc_contains(i.entry->item, bb4);
			if (contained)
				assert(jive_cfg_scc_contains(i.entry->item, bb6));
			else {
				assert(jive_cfg_scc_contains(i.entry->item, bb5));
				assert(jive_cfg_scc_contains(i.entry->item, bb7));
			}
		}
	}
}

static int
test_main(void)
{
	jive_context * context = jive_context_create();
	jive_clg * clg = jive_clg_create(context);
	jive_clg_node * clg_node = jive_clg_node_create(clg, "foobar");
	clg_node->cfg = jive_cfg_create(clg_node);

	setup_cfg(clg_node->cfg);

//	jive_cfg_view(clg_node->cfg);

	jive_cfg_scc_set * scc_set = jive_cfg_scc_set_create(context);
	jive_cfg_find_sccs(clg_node->cfg, scc_set);
	check_sccs(scc_set);

	jive_cfg_scc_set_destroy(scc_set);
	jive_clg_destroy(clg);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("frontend/test-cfg-scc", test_main);
