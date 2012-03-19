#ifndef JIVE_ARCH_INSTRUCTIONSET_H
#define JIVE_ARCH_INSTRUCTIONSET_H

#include <jive/arch/transfer-instructions.h>
#include <jive/common.h>

struct jive_input;
struct jive_instruction_class;
struct jive_node;
struct jive_output;
struct jive_reg_classifier;

typedef struct jive_instructionset_class jive_instructionset_class;
typedef struct jive_instructionset jive_instructionset;
typedef struct jive_xfer_description jive_xfer_description;

struct jive_xfer_description {
	struct jive_input * input;
	struct jive_node * node;
	struct jive_output * output;
};

struct jive_instructionset_class {
	jive_xfer_block (*create_xfer)(struct jive_region * region, struct jive_output * origin,
		const struct jive_resource_class * in_class, const struct jive_resource_class * out_class);
};

struct jive_instructionset {
	const jive_instructionset_class * class_;
	const struct jive_instruction_class * jump_instruction_class;
	const struct jive_reg_classifier * reg_classifier;
};

JIVE_EXPORTED_INLINE jive_xfer_description
jive_instructionset_create_xfer(const jive_instructionset * self,
	struct jive_region * region, struct jive_output * origin,
	const struct jive_resource_class * in_class, const struct jive_resource_class * out_class)
{
	jive_xfer_description desc;
	jive_xfer_block block = self->class_->create_xfer(region, origin, in_class, out_class);
	desc.input = block.input;
	desc.node = block.node;
	desc.output = block.output;
	return desc;
}

JIVE_EXPORTED_INLINE const struct jive_instruction_class *
jive_instructionset_get_jump_instruction_class(const jive_instructionset * self)
{
	return self->jump_instruction_class;
}

JIVE_EXPORTED_INLINE const struct jive_reg_classifier *
jive_instructionset_get_reg_classifier(const jive_instructionset * self)
{
	return self->reg_classifier;
}

#endif
