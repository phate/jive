/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SEQTYPE_H
#define JIVE_VSDG_SEQTYPE_H

#include <jive/vsdg/statetype.h>

namespace jive {
namespace seq {

class type final : public jive::state::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::state::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::seq::type * copy() const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

class output final : public jive::state::output {
public:
	virtual ~output() noexcept;

	output(struct jive_node * node, size_t index);

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::seq::type type_;
};

class gate final : public jive::state::gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::seq::type &
	type() const noexcept override
	{
		return type_;
	}

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::seq::type type_;
};

const type seqtype;

}
}

#endif
