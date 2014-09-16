/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_TYPE_H
#define JIVE_ARCH_IMMEDIATE_TYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace imm {

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::value::type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::imm::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

class input final : public jive::value::input {
public:
	virtual ~input() noexcept;

	input(struct jive_node * node, size_t index, jive::output * origin);

	virtual const jive::imm::type & type() const noexcept { return type_; }

private:
	input(const input & rhs) = delete;
	input& operator=(const input & rhs) = delete;

	jive::imm::type type_;
};

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(jive_node * node, size_t index);

	virtual const jive::imm::type & type() const noexcept { return type_; }

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::imm::type type_;
};

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::imm::type & type() const noexcept { return type_; }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::imm::type type_;
};

}
}

#endif
