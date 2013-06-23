/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/util/stack.h>

#include <assert.h>

JIVE_DECLARE_STACK_TYPE(my_stack, int);
JIVE_DEFINE_STACK_TYPE(my_stack, int);

static int
test_main(void)
{
	jive_context * context = jive_context_create();

	struct my_stack stack;
	my_stack_init(&stack, context);

	assert(my_stack_size(&stack) == 0);

	my_stack_pop(&stack);

	my_stack_push(&stack, 3);
	my_stack_push(&stack, 42);
	my_stack_push(&stack, 1);
	assert(my_stack_size(&stack) == 3);
	assert(my_stack_top(&stack) == 1);

	my_stack_pop(&stack);
	assert(my_stack_size(&stack) == 2);
	assert(my_stack_top(&stack) == 42);
	
	my_stack_pop(&stack);
	assert(my_stack_size(&stack) == 1);
	assert(my_stack_top(&stack) == 3);
	
	my_stack_pop(&stack);
	assert(my_stack_size(&stack) == 0);
	my_stack_pop(&stack);

	my_stack_fini(&stack);
	jive_context_assert_clean(context);
	jive_context_destroy(context);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-stack", test_main);
