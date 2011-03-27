#include <assert.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/vsdg/resource-private.h>

const jive_register_class 
	gpr = {
		.base = {
			.name = "gpr",
			.limit = 4,
			.names = NULL,
			.parent = &jive_root_resource_class,
			.depth = 1,
		},
		.regs = NULL
	},
	evenreg = {
		.base = {
			.name = "even",
			.limit = 2,
			.names = NULL,
			.parent = &gpr.base,
			.depth = 2,
		},
		.regs = NULL
	},
	oddreg = {
		.base = {
			.name = "odd",
			.limit = 2,
			.names = NULL,
			.parent = &gpr.base,
			.depth = 2,
		},
		.regs = NULL
	},
	reg0 = {
		.base = {
			.name = "reg0",
			.limit = 1,
			.names = NULL,
			.parent = &evenreg.base,
			.depth = 3,
		},
		.regs = NULL
	},
	reg1 = {
		.base = {
			.name = "reg1",
			.limit = 1,
			.names = NULL,
			.parent = &oddreg.base,
			.depth = 3,
		},
		.regs = NULL
	},
	reg2 = {
		.base = {
			.name = "reg2",
			.limit = 1,
			.names = NULL,
			.parent = &evenreg.base,
			.depth = 3,
		},
		.regs = NULL
	},
	reg3 = {
		.base = {
			.name = "reg3",
			.limit = 1,
			.names = NULL,
			.parent = &oddreg.base,
			.depth = 3,
		},
		.regs = NULL
	}
;

void test_regcls_count(jive_context * ctx)
{
	jive_resource_class_count count;
	jive_resource_class_count_init(&count);
	
	const jive_resource_class * overflow;
	
	overflow = jive_resource_class_count_add(&count, ctx, &reg0.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &reg1.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &reg0.base);
	assert(overflow);
	
	overflow = jive_resource_class_count_add(&count, ctx, &evenreg.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &reg2.base);
	assert(overflow == &evenreg.base);
	
	overflow = jive_resource_class_count_check_change(&count, &evenreg.base, &oddreg.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_change(&count, &evenreg.base, &reg2.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_change(&count, &evenreg.base, &reg0.base);
	assert(overflow == &reg0.base);
	
	jive_resource_class_count_fini(&count, ctx);
}

int main()
{
	jive_context * ctx = jive_context_create();
	
	test_regcls_count(ctx);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
