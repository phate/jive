/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_INSTRUCTIONSET_H
#define JIVE_ARCH_INSTRUCTIONSET_H

#include <jive/common.h>

namespace jive {
	class input;
	class instruction;
	class node;
	class output;
	class region;
	class resource_class;
}

namespace jive {

class register_classifier;

class xfer_description {
public:
	inline
	xfer_description(
		jive::input * input,
		jive::node * node,
		jive::output * output)
	: node_(node)
	, input_(input)
	, output_(output)
	{}

	inline jive::input *
	input() const noexcept
	{
		return input_;
	}

	inline jive::node *
	node() const noexcept
	{
		return node_;
	}

	inline jive::output *
	output() const noexcept
	{
		return output_;
	}

private:
	jive::node * node_;
	jive::input * input_;
	jive::output * output_;
};

class instructionset {
public:
	virtual
	~instructionset();

	inline constexpr
	instructionset()
	{}

	virtual const instruction *
	jump_instruction() const noexcept = 0;

	virtual const register_classifier *
	classifier() const noexcept = 0;

	virtual xfer_description
	create_xfer(
		jive::region * region,
		jive::output * origin,
		const jive::resource_class * in_class,
		const jive::resource_class * out_class) = 0;
};

}

#endif
