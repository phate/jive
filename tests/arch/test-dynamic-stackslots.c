#include <assert.h>

#include <jive/arch/stackslot.h>

int main()
{
	const jive_resource_class * cls1, * cls2;
	
	cls1 = jive_stackslot_size_class_get(8, 8);
	assert(cls1 == &jive_stackslot_class_8_8.base);
	
	cls1 = jive_stackslot_size_class_get(16, 16);
	assert(cls1 == &jive_stackslot_class_16_16.base);
	
	cls1 = jive_stackslot_size_class_get(32, 32);
	assert(cls1 == &jive_stackslot_class_32_32.base);
	
	cls1 = jive_stackslot_size_class_get(64, 64);
	assert(cls1 == &jive_stackslot_class_64_64.base);
	
	cls1 = jive_stackslot_size_class_get(64, 32);
	assert(cls1);
	cls2 = jive_stackslot_size_class_get(64, 32);
	assert(cls1 == cls2);
	
	cls1 = jive_fixed_stackslot_class_get(32, 0);
	assert(cls1->parent == &jive_stackslot_class_32_32.base);
	assert(cls1->limit == 1 && cls1->names[0] != 0);
	
	cls2 = jive_fixed_stackslot_class_get(32, 0);
	assert(cls1 == cls2);
	
	cls2 = jive_fixed_stackslot_class_get(32, 4);
	assert(cls2->parent == &jive_stackslot_class_32_32.base);

	cls2 = jive_fixed_stackslot_class_get(32, -4);
	assert(cls2->parent == &jive_stackslot_class_32_32.base);
	
	cls2 = jive_fixed_stackslot_class_get(32, 2);
	assert(cls2->parent != &jive_stackslot_class_32_32.base);
	
	return 0;
}
