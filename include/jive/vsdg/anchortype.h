/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHORTYPE_H
#define JIVE_VSDG_ANCHORTYPE_H

#include <jive/vsdg/basetype.h>

namespace jive {
namespace achr {

class type final : public jive::base::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::base::type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::achr::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

class input final : public jive::input {
public:
	virtual ~input() noexcept;

	input(struct jive_node * node, size_t index, jive::output * origin);

	virtual const jive::achr::type & type() const noexcept { return type_; }

private:
	jive::achr::type type_;
};

class output final : public jive::output {
public:
	virtual ~output() noexcept;

	output(struct jive_node * node, size_t index);

	virtual const jive::achr::type & type() const noexcept { return type_; }

private:
	jive::achr::type type_;
};

}
}

#endif
