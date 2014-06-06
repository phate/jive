/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESSTYPE_H
#define JIVE_ARCH_ADDRESSTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace addr {

/* address type */

class type final : public jive_value_type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive_value_type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual jive::addr::type * copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

/* address input */

class input final : public jive_value_input {
public:
	virtual ~input() noexcept;

	input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive::addr::type & type() const noexcept { return type_; }

private:
	jive::addr::type type_;
};

/* address output */

class output final : public jive_value_output {
public:
	virtual ~output() noexcept;

	output(jive_node * node, size_t index);

	virtual const jive::addr::type & type() const noexcept { return type_; }

private:
	jive::addr::type type_;
};

/* address gate */

class gate final : public jive_value_gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::addr::type & type() const noexcept { return type_; }

private:
	jive::addr::type type_;
};

}
}

#endif
