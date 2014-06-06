/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/statetype.h>

namespace jive {
namespace mem {

class type final : public jive_state_type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive_state_type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual jive::mem::type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

class input final : public jive_state_input {
public:
	virtual ~input() noexcept;

	input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive::mem::type & type() const noexcept { return type_; }

private:
	jive::mem::type type_;
};

class output final : public jive_state_output {
public:
	virtual ~output() noexcept;

	output(jive_node * node, size_t index);

	virtual const jive::mem::type & type() const noexcept { return type_; }

private:
	jive::mem::type type_;
};

class gate final : public jive_state_gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::mem::type & type() const noexcept { return type_; }

private:
	jive::mem::type type_;
};

}
}

#endif
