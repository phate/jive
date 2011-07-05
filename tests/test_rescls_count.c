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

void test_rescls_count_addsub(jive_context * ctx)
{
	jive_resource_class_count count;
	jive_resource_class_count_init(&count, ctx);
	
	const jive_resource_class * overflow;
	
	overflow = jive_resource_class_count_add(&count, &reg0.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &reg1.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &reg0.base);
	assert(overflow);
	
	overflow = jive_resource_class_count_add(&count, &evenreg.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &reg2.base);
	assert(overflow == &evenreg.base);
	
	overflow = jive_resource_class_count_check_change(&count, &evenreg.base, &oddreg.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_change(&count, &evenreg.base, &reg2.base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_change(&count, &evenreg.base, &reg0.base);
	assert(overflow == &reg0.base);
	
	jive_resource_class_count_fini(&count);
}

void test_rescls_count_compound(jive_context * ctx)
{
	jive_resource_class_count a, b, c;
	jive_resource_class_count_init(&a, ctx);
	jive_resource_class_count_init(&b, ctx);
	jive_resource_class_count_init(&c, ctx);
	
	assert(jive_resource_class_count_equals(&a, &b));
	
	jive_resource_class_count_add(&a, &reg0.base);
	jive_resource_class_count_add(&a, &reg1.base);
	jive_resource_class_count_add(&b, &reg0.base);
	assert(!jive_resource_class_count_equals(&a, &b));
	
	jive_resource_class_count_copy(&c, &b);
	assert(jive_resource_class_count_equals(&b, &c));
	
	jive_resource_class_count_add(&c, &reg1.base);
	assert(jive_resource_class_count_equals(&a, &c));
	
	jive_resource_class_count_clear(&a);
	jive_resource_class_count_clear(&b);
	jive_resource_class_count_clear(&c);
	
	jive_resource_class_count_add(&a, &reg0.base);
	jive_resource_class_count_add(&a, &reg1.base);
	jive_resource_class_count_add(&b, &reg1.base);
	jive_resource_class_count_add(&b, &reg2.base);
	jive_resource_class_count_update_intersection(&a, &b);
	jive_resource_class_count_add(&c, &reg1.base);
	jive_resource_class_count_add(&c, &evenreg.base);
	assert(a.nitems == 5);
	assert(jive_resource_class_count_equals(&a, &c));
	
	jive_resource_class_count_clear(&a);
	jive_resource_class_count_clear(&b);
	jive_resource_class_count_clear(&c);
	
	jive_resource_class_count_add(&a, &reg0.base);
	jive_resource_class_count_add(&a, &reg1.base);
	jive_resource_class_count_add(&b, &reg1.base);
	jive_resource_class_count_add(&b, &reg2.base);
	jive_resource_class_count_update_union(&a, &b);
	jive_resource_class_count_add(&c, &reg0.base);
	jive_resource_class_count_add(&c, &reg1.base);
	jive_resource_class_count_add(&c, &reg2.base);
	jive_resource_class_count_sub(&c, &evenreg.base);
	assert(a.nitems == 7);
	assert(jive_resource_class_count_equals(&a, &c));
	
	jive_resource_class_count_fini(&a);
	jive_resource_class_count_fini(&b);
	jive_resource_class_count_fini(&c);
}

int main()
{
	jive_context * ctx = jive_context_create();
	
	test_rescls_count_addsub(ctx);
	test_rescls_count_compound(ctx);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
