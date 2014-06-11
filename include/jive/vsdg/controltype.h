/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_CONTROLTYPE_H
#define JIVE_VSDG_CONTROLTYPE_H

#include <jive/vsdg/statetype.h>

namespace jive {
namespace ctl {

class type final : public jive::state::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::state::type() {};

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::ctl::type * copy() const override;

	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

class input final : public jive::state::input {
public:
	virtual ~input() noexcept;

	input(struct jive_node * node, size_t index, jive::output * initial_operand);

	virtual const jive::ctl::type & type() const noexcept { return type_; }

private:
	jive::ctl::type type_;
};

class output final : public jive::state::output {
public:
	virtual ~output() noexcept;

	output(bool active, struct jive_node * node, size_t index);

	virtual const jive::ctl::type & type() const noexcept { return type_; }

	inline void set_active(bool active) noexcept { active_ = active; }

	inline bool active() const noexcept { return active_; }

private:
	/*
		FIXME: this is an ugly hack, introduce a new type for that
	*/
	bool active_;
	jive::ctl::type type_;
};

class gate final : public jive::state::gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::ctl::type & type() const noexcept { return type_; }

private:
	jive::ctl::type type_;
};

}
}

#endif
