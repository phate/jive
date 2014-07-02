/*
 * Copyright 2013 2014 Nico Reissmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/frontend/cfg.h>
#include <jive/context.h>
#include <jive/frontend/basic_block.h>
#include <jive/frontend/tac/bitstring/arithmetic/bitsum.h>
#include <jive/frontend/tac/variable.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

static int test_main(void)
{
	jive_context * context = jive_context_create();
	jive_clg * clg = jive_clg_create(context);
	jive_clg_node * clg_node = jive_clg_node_create(clg, "foobar");
	clg_node->cfg = jive_cfg_create(clg_node);

	jive_basic_block * basic_block = jive_basic_block_cast(jive_basic_block_create(clg_node->cfg));

	jive_three_address_code * tac1 = jive_variable_code_create(basic_block, "blub");
	jive_three_address_code * tac2 = jive_variable_code_create(basic_block, "foobar");

	jive_bitsum_code_create(basic_block, tac1, tac2);

//	jive_cfg_view(cfg);

	jive_clg_destroy(clg);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("frontend/tac/bitstring/arithmetic/test-bitsum", test_main);
