#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

typedef struct jive_resource_class jive_resource_class;
typedef struct jive_resource_name jive_resource_name;

const jive_resource_class *
jive_resource_class_intersection(const jive_resource_class * self, const jive_resource_class * other);

extern const jive_resource_class jive_root_resource_class;

struct jive_resource_name {
	const jive_resource_class * resource_class;
};

#endif
