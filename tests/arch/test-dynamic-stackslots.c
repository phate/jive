#include <assert.h>

#include <jive/arch/stackslot.h>

int main()
{
	const jive_resource_class * cls1, * cls2;
	
	cls1 = jive_stackslot_size_class_get(1, 1);
	assert(cls1 == &jive_stackslot_class_1_1.base);
	
	cls1 = jive_stackslot_size_class_get(2, 2);
	assert(cls1 == &jive_stackslot_class_2_2.base);
	
	cls1 = jive_stackslot_size_class_get(4, 4);
	assert(cls1 == &jive_stackslot_class_4_4.base);
	
	cls1 = jive_stackslot_size_class_get(8, 8);
	assert(cls1 == &jive_stackslot_class_8_8.base);
	
	cls1 = jive_stackslot_size_class_get(8, 4);
	assert(cls1);
	cls2 = jive_stackslot_size_class_get(8, 4);
	assert(cls1 == cls2);
	
	cls1 = jive_fixed_stackslot_class_get(4, 4, 0);
	assert(cls1->parent == &jive_stackslot_class_4_4.base);
	assert(cls1->limit == 1 && cls1->names[0] != 0);
	
	cls2 = jive_fixed_stackslot_class_get(4, 4, 0);
	assert(cls1 == cls2);
	
	cls2 = jive_fixed_stackslot_class_get(4, 4, 4);
	assert(cls2->parent == &jive_stackslot_class_4_4.base);

	cls2 = jive_fixed_stackslot_class_get(4, 4, -4);
	assert(cls2->parent == &jive_stackslot_class_4_4.base);
	
	cls2 = jive_fixed_stackslot_class_get(4, 2, 2);
	assert(cls2->parent != &jive_stackslot_class_4_4.base);
	
	return 0;
}
