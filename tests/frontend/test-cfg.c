/*
 * Copyright 2014 Nico Reissmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/frontend/basic_block.h>
#include <jive/frontend/cfg.h>

#include <assert.h>

static void
test_invalid0_cfg()
{
	jive::frontend::cfg cfg;
	
	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 0);
	bb1->add_outedge(cfg.exit(), 1);
	
//	jive_cfg_view(cfg);

	assert(!cfg.is_valid());
}

static void
test_invalid1_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(cfg.exit(), 0);
	bb1->add_outedge(bb2, 0);
	bb2->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(!cfg.is_valid());
}

static void
test_invalid2_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(cfg.exit(), 0);
	bb1->add_outedge(cfg.exit(), 1);

//	jive_cfg_view(cfg);

	assert(!cfg.is_valid());
}

static void
test_notclosed_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();

	bb1->add_outedge(bb2, 0);
	bb2->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(!cfg.is_closed());

	cfg.prune();
	
	assert(cfg.is_closed());
}

static void
test_linear_cfg()
{
	jive::frontend::cfg cfg;
	
	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 0);
	bb2->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(cfg.is_linear());
	assert(cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_ifthen_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 1);
	bb1->add_outedge(cfg.exit(), 0);
	bb2->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_ifthenelse_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();
	jive::frontend::basic_block * bb3 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 1);
	bb1->add_outedge(bb3, 0);
	bb2->add_outedge(cfg.exit(), 0);
	bb3->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_switch_cfg()
{
	jive::frontend::cfg cfg;
	
	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();
	jive::frontend::basic_block * bb3 = cfg.create_basic_block();
	jive::frontend::basic_block * bb4 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 0);
	bb1->add_outedge(bb3, 1);
	bb1->add_outedge(bb4, 2);
	bb1->add_outedge(cfg.exit(), 3);
	bb2->add_outedge(cfg.exit(), 0);
	bb3->add_outedge(cfg.exit(), 0);
	bb4->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_dowhile_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb1, 1);
	bb1->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_while_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 1);
	bb2->add_outedge(bb1, 0);
	bb1->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());	
	assert(!cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_unstructured0_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();
	jive::frontend::basic_block * bb3 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 0);
	bb1->add_outedge(bb3, 1);
	bb2->add_outedge(bb3, 0);
	bb2->add_outedge(cfg.exit(), 1);
	bb3->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(!cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_unstructured1_cfg()
{
	/*
		for (...) {
			if (...) {
				continue;
			}
		}
	*/

	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();
	jive::frontend::basic_block * bb3 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(cfg.exit(), 1);
	bb1->add_outedge(bb2, 0);
	bb2->add_outedge(bb3, 1);
	bb2->add_outedge(bb1, 0);
	bb3->add_outedge(bb1, 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(!cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_unstructured2_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();
	jive::frontend::basic_block * bb3 = cfg.create_basic_block();
	jive::frontend::basic_block * bb4 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb4, 0);
	bb1->add_outedge(bb2, 1);
	bb2->add_outedge(bb4, 0);
	bb2->add_outedge(bb3, 1);
	bb3->add_outedge(bb4, 0);
	bb4->add_outedge(cfg.exit(), 0);

//	jive_cfg_view(cfg);

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(!cfg.is_structured());
	assert(cfg.is_reducible());
}

static void
test_irreducible_cfg()
{
	jive::frontend::cfg cfg;

	jive::frontend::basic_block * bb1 = cfg.create_basic_block();
	jive::frontend::basic_block * bb2 = cfg.create_basic_block();
	jive::frontend::basic_block * bb3 = cfg.create_basic_block();
	jive::frontend::basic_block * bb4 = cfg.create_basic_block();
	jive::frontend::basic_block * bb5 = cfg.create_basic_block();

	cfg.exit()->divert_inedges(bb1);
	bb1->add_outedge(bb2, 0);
	bb1->add_outedge(bb3, 1);
	bb2->add_outedge(bb4, 0);
	bb2->add_outedge(bb5, 1);
	bb3->add_outedge(bb5, 0);
	bb3->add_outedge(bb4, 1);
	bb4->add_outedge(bb3, 0);
	bb4->add_outedge(cfg.exit(), 1);
	bb5->add_outedge(bb2, 0);
	bb5->add_outedge(cfg.exit(), 1);

//	jive_cfg_view(cfg);

	cfg.prune();

	assert(cfg.is_valid());
	assert(cfg.is_closed());
	assert(!cfg.is_linear());
	assert(!cfg.is_structured());
	assert(!cfg.is_reducible());
}

static int
test_main(void)
{
	test_invalid0_cfg();
	test_invalid1_cfg();
	test_invalid2_cfg();
	test_notclosed_cfg();
	test_linear_cfg();
	test_ifthen_cfg();
	test_ifthenelse_cfg();
	test_switch_cfg();
	test_dowhile_cfg();
	test_while_cfg();
	test_unstructured0_cfg();
	test_unstructured1_cfg();
	test_unstructured2_cfg();
	test_irreducible_cfg();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("frontend/test-cfg", test_main);
