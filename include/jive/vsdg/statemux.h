/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STATEMUX_H
#define JIVE_VSDG_STATEMUX_H

#include <jive/vsdg/simple_node.h>

namespace jive {

class mux_op final : public simple_op {
public:
	virtual
	~mux_op() noexcept;

	inline
	mux_op(const state::type & state_type, size_t narguments, size_t nresults)
	: port_(state_type)
	, nresults_(nresults)
	, narguments_(narguments)
	{}

	inline
	mux_op(const mux_op &) = default;

	inline
	mux_op(mux_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port port_;
	size_t nresults_;
	size_t narguments_;
};

static inline jive::output *
create_state_merge(
	const jive::state::type & type,
	const std::vector<jive::output*> & states)
{
	if (states.empty())
		throw jive::compiler_error("Insufficient number of operands.");

	auto region = states.front()->region();
	jive::mux_op op(type, states.size(), 1);
	return jive::create_normalized(region, op, states)[0];
}

static inline std::vector<jive::output*>
create_state_split(
	const jive::state::type & type,
	jive::output * state,
	size_t nresults)
{
	jive::mux_op op(type, 1, nresults);
	return jive::create_normalized(state->region(), op, {state});
}

}

#endif
