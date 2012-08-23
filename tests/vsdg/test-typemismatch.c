#include <assert.h>
#include <locale.h>
#include <setjmp.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static void jump(void * where, const char * msg)
{
	jmp_buf * buffer=(jmp_buf *)where;
	longjmp(*buffer, 1);
}

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	JIVE_DECLARE_TYPE(type);
	JIVE_DECLARE_VALUE_TYPE(value_type);
	
	jive_node * n1 = jive_node_create(region,
		0, NULL, NULL,
		1, (const jive_type *[]){type});
	
	bool error_handler_called = false;
	
	jmp_buf buffer;
	if (setjmp(buffer) == 0) {
		jive_set_fatal_error_handler(ctx, jump, &buffer);
		jive_node_create(region,
			1, (const jive_type *[]){value_type}, (jive_output *[]){n1->outputs[0]},
			0, 0);
	} else {
		error_handler_called = true;
	}
	
	assert(error_handler_called);
	
	jive_context_destroy(ctx);
	
	return 0;
}
