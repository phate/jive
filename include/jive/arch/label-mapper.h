/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LABEL_MAPPER_H
#define JIVE_ARCH_LABEL_MAPPER_H

#include <jive/vsdg/label.h>

typedef struct jive_label_name_mapper jive_label_name_mapper;
typedef struct jive_label_name_mapper_class jive_label_name_mapper_class;

struct jive_label_name_mapper_class {
	void (*destroy)(jive_label_name_mapper * self);
	const char * (*map_label_internal)(jive_label_name_mapper * self, const jive_label_internal * label);
	const char * (*map_label_external)(jive_label_name_mapper * self, const jive_label_external * label);
};

struct jive_label_name_mapper {
	const jive_label_name_mapper_class * class_;
};

JIVE_EXPORTED_INLINE void
jive_label_name_mapper_destroy(jive_label_name_mapper * self)
{
	self->class_->destroy(self);
}

JIVE_EXPORTED_INLINE const char *
jive_label_name_mapper_map_label_internal(
	jive_label_name_mapper * self,
	const jive_label_internal * label)
{
	return self->class_->map_label_internal(self, label);
}

JIVE_EXPORTED_INLINE const char *
jive_label_name_mapper_map_label_external(
	jive_label_name_mapper * self,
	const jive_label_external * label)
{
	return self->class_->map_label_external(self, label);
}

const char *
jive_label_name_mapper_map_label(
	jive_label_name_mapper * self,
	const jive_label * label);

jive_label_name_mapper *
jive_label_name_mapper_simple_create(jive_context * context);

#endif
