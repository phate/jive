/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTIONSET_H
#define JIVE_ARCH_INSTRUCTIONSET_H

#include <jive/common.h>

namespace jive {
	class instruction_class;
	class node;
	class region;
	class resource_class;
	class simple_input;
	class simple_output;
}

class jive_reg_classifier;

typedef struct jive_xfer_description jive_xfer_description;

struct jive_xfer_description {
	jive::simple_input * input;
	jive::node * node;
	jive::simple_output * output;
};

namespace jive {

class instructionset {
public:
	virtual
	~instructionset();

	inline constexpr
	instructionset()
	{}

	virtual const instruction_class *
	jump_instruction() const noexcept = 0;

	virtual const jive_reg_classifier *
	classifier() const noexcept = 0;

	virtual jive_xfer_description
	create_xfer(
		jive::region * region,
		jive::simple_output * origin,
		const jive::resource_class * in_class,
		const jive::resource_class * out_class) = 0;
};

}

#endif
