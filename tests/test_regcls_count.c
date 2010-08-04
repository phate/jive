#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/arch/registers.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/regcls-count-private.h>
#include <jive/vsdg/valuetype-private.h>

const jive_regcls
	gpr = {
		.parent = 0,
		.nregs = 4,
		.depth = 0,
		.name = "gpr"
	},
	evenreg = {
		.parent = &gpr,
		.nregs = 2,
		.depth = 1,
		.name = "even"
	},
	oddreg = {
		.parent = &gpr,
		.nregs = 2,
		.depth = 1,
		.name = "odd"
	},
	reg0 = {
		.parent = &evenreg,
		.nregs = 1,
		.depth = 2,
		.name = "reg0"
	},
	reg1 = {
		.parent = &oddreg,
		.nregs = 1,
		.depth = 2,
		.name = "reg1"
	},
	reg2 = {
		.parent = &evenreg,
		.nregs = 1,
		.depth = 2,
		.name = "reg2"
	},
	reg3 = {
		.parent = &oddreg,
		.nregs = 1,
		.depth = 2,
		.name = "reg3"
	};

void test_regcls_count(jive_context * ctx)
{
	jive_regcls_count count;
	jive_regcls_count_init(&count);
	
	const jive_regcls * overflow;
	
	overflow = jive_regcls_count_add(&count, ctx, &reg0);
	assert(!overflow);
	
	overflow = jive_regcls_count_check_add(&count, &reg1);
	assert(!overflow);
	
	overflow = jive_regcls_count_check_add(&count, &reg0);
	assert(overflow);
	
	overflow = jive_regcls_count_add(&count, ctx, &evenreg);
	assert(!overflow);
	
	overflow = jive_regcls_count_check_add(&count, &reg2);
	assert(overflow == &evenreg);
	
	overflow = jive_regcls_count_check_change(&count, &evenreg, &oddreg);
	assert(!overflow);
	
	overflow = jive_regcls_count_check_change(&count, &evenreg, &reg2);
	assert(!overflow);
	
	overflow = jive_regcls_count_check_change(&count, &evenreg, &reg0);
	assert(overflow == &reg0);
	
	jive_regcls_count_fini(&count, ctx);
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	test_regcls_count(ctx);
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	
	JIVE_DECLARE_VALUE_TYPE(type);
	
	jive_node * top = jive_node_create(region,
		0, NULL, NULL,
		2, (const jive_type *[]){type, type});
	
	jive_node * mid = jive_node_create(region,
		1, (const jive_type *[]){type, }, (jive_output *[]){top->outputs[0]},
		1, (const jive_type *[]){type});
	
	jive_node * bottom = jive_node_create(region,
		2, (const jive_type *[]){type, type}, (jive_output *[]){mid->outputs[0], top->outputs[1]},
		0, NULL);
	
	jive_resource * r1 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r1, top->outputs[1]);
	jive_resource_assign_input(r1, bottom->inputs[1]);
	
	jive_resource * r2 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r2, top->outputs[0]);
	jive_resource_assign_input(r2, mid->inputs[0]);
	
	jive_resource * r3 = jive_type_create_resource(type, graph);
	jive_resource_assign_output(r3, mid->outputs[0]);
	jive_resource_assign_input(r3, bottom->inputs[0]);
	
	jive_value_resource_set_regcls((jive_value_resource *) r1, &reg0);
	jive_value_resource_set_regcls((jive_value_resource *) r2, &reg1);
	assert(jive_resource_can_merge(r2, r3));
	jive_value_resource_set_regcls((jive_value_resource *) r3, &reg1);
	
	assert(jive_value_resource_check_change_regcls((jive_value_resource *)r2, &reg2) == 0);
	assert(jive_value_resource_check_change_regcls((jive_value_resource *)r2, &reg0) == &reg0);
	
	const jive_regcls * overflow = jive_regcls_count_check_add(&top->use_count_after, &reg0);
	assert(overflow == &reg0);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
