/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTIONSET_H
#define JIVE_ARCH_INSTRUCTIONSET_H

#include <jive/common.h>

namespace jive {
	class input;
}

struct jive_instruction_class;
struct jive_node;
struct jive_output;
struct jive_reg_classifier;
struct jive_region;
struct jive_resource_class;

typedef struct jive_instructionset_class jive_instructionset_class;
typedef struct jive_instructionset jive_instructionset;
typedef struct jive_xfer_description jive_xfer_description;

struct jive_xfer_description {
	jive::input * input;
	struct jive_node * node;
	struct jive_output * output;
};

struct jive_instructionset_class {
	jive_xfer_description (*create_xfer)(struct jive_region * region, struct jive_output * origin,
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
	return  self->class_->create_xfer(region, origin, in_class, out_class);
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
