/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGISTERS_H
#define JIVE_ARCH_REGISTERS_H

#include <jive/vsdg/resource.h>

struct jive_graph;

namespace jive {
namespace base {
	class type;
}
}

typedef struct jive_register_class jive_register_class;
typedef struct jive_register_name jive_register_name;

struct jive_register_name {
	jive_resource_name base;
	
	int code;
};

struct jive_register_class {
	jive_resource_class base;
	
	//const jive_register_name * regs;
	
	size_t nbits;
	size_t int_arithmetic_width;
	size_t loadstore_width;
};

const struct jive::base::type *
jive_register_class_get_type(const jive_register_class * self);

extern const jive_resource_class jive_root_register_class;
extern const jive_resource_class_class JIVE_REGISTER_RESOURCE;

#endif
