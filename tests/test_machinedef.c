#include <assert.h>
#include <jive/machine.h>
#include <jive/i386/machine.h>

static bool register_in_class(const jive_cpureg * reg, const jive_cpureg_class * regcls)
{
	size_t n;
	for(n=0; n<regcls->nregs; n++)
		if (reg == &regcls->regs[n]) return true;
	return false;
}

static void verify_register_def(const jive_machine * machine, const jive_cpureg * reg)
{
	size_t n;
	assert(register_in_class(reg, reg->regcls));
	assert(reg->class_mask == reg->regcls->class_mask);
	assert(reg->class_mask & (1 << reg->regcls->index));
	for(n=0; n<machine->nregcls; n++) {
		const jive_cpureg_class * regcls = &machine->regcls[n];
		if (reg->class_mask & (1<<n)) {
			assert(register_in_class(reg, regcls));
			assert((reg->class_mask & regcls->class_mask) == regcls->class_mask);
			assert((reg->class_mask | regcls->class_mask) == reg->class_mask);
		}
	}
	
	jive_cpureg_classmask_t mask = 0;
	const jive_cpureg_class * regcls = reg->regcls;
	while(regcls) {
		mask = mask | (1 << regcls->index);
		regcls = regcls->parent;
	}
	
	assert(mask);
	assert(mask == reg->class_mask);
}

/* of two register classes, either one must contain the other or they
must be disjoint */
static void verify_regcls_inclusion(const jive_cpureg_class * cls1, const jive_cpureg_class * cls2)
{
	bool overlap = (cls1->regs < cls2->regs+cls2->nregs) && (cls2->regs < cls1->regs+cls1->nregs);
	bool contains1 = (cls1->regs >= cls2->regs) && (cls1->regs+cls1->nregs <= cls2->regs+cls2->nregs);
	bool contains2 = (cls2->regs >= cls1->regs) && (cls2->regs+cls2->nregs <= cls1->regs+cls1->nregs);
	
	assert (!overlap || (contains1 || contains2));
}

static void verify_regcls_def(const jive_machine * machine, const jive_cpureg_class * regcls)
{
	assert(regcls->class_mask & (1 << regcls->index));
	if (regcls->parent) {
		assert((regcls->parent->class_mask & regcls->class_mask) == regcls->parent->class_mask); 
		assert((regcls->parent->class_mask | regcls->class_mask) == regcls->class_mask); 
		assert(regcls->nbits == regcls->parent->nbits);
		assert(regcls->int_arithmetic_width == regcls->parent->int_arithmetic_width);
		assert(regcls->float_arithmetic_width == regcls->parent->float_arithmetic_width);
		assert(regcls->loadstore_width == regcls->parent->loadstore_width);
	}
	size_t n;
	for(n=0; n<regcls->nregs; n++) {
		const jive_cpureg * reg = &regcls->regs[n];
		assert((reg->class_mask & regcls->class_mask) == regcls->class_mask);
		assert((reg->class_mask | regcls->class_mask) == reg->class_mask);
	}
	
	jive_cpureg_classmask_t mask = 0;
	const jive_cpureg_class * tmp = regcls;
	while(tmp) {
		mask = mask | (1 << tmp->index);
		tmp = tmp->parent;
	}
	
	assert(mask);
	assert(mask == regcls->class_mask);
	
	for(n=0; n<machine->nregcls; n++)
		verify_regcls_inclusion(regcls, &machine->regcls[n]);
}

static void verify_regcls_budget(const jive_machine * machine)
{
	jive_regcls_count total;
	jive_regcls_count_init(total);
	size_t n;
	for(n=0; n<machine->nregs; n++) {
		jive_regcls_count_add(total, machine->regs[n].regcls);
	}
	for(n=0; n<MAX_REGISTER_CLASSES; n++)
		assert(total[n] == machine->regcls_budget[n]);
}

static void verify_machinedef(const jive_machine * machine)
{
	size_t n;
	for(n=0; n<machine->nregs; n++)
		verify_register_def(machine, &machine->regs[n]);
	for(n=0; n<machine->nregcls; n++)
		verify_regcls_def(machine, &machine->regcls[n]);
	verify_regcls_budget(machine);
}

int main()
{
	verify_machinedef(&jive_i386_machine);
	return 0;
}
