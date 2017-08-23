/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STATETYPE_H
#define JIVE_VSDG_STATETYPE_H

#include <memory>
#include <vector>

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/simple.h>

namespace jive {
namespace state {

class mux_op final : public simple_op {
public:
	virtual
	~mux_op() noexcept;

	inline
	mux_op(const type & state_type, size_t narguments, size_t nresults)
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

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

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

}
}

jive::output*
jive_state_merge(
	const jive::state::type * statetype,
	size_t nstates,
	jive::output * const states[]);

std::vector<jive::output*>
jive_state_split(
	const jive::state::type * statetype,
	jive::output * state,
	size_t nstates);


// FIXME: temporary overloads below, until it has been made syntactically sure
// that only state types are passed as arguments
static inline jive::output*
jive_state_merge(
	const jive::base::type * statetype,
	size_t nstates,
	jive::output * const states[])
{
	return jive_state_merge(&dynamic_cast<const jive::state::type &>(*statetype), nstates, states);
}

static inline std::vector<jive::output*>
jive_state_split(
	const jive::base::type * statetype,
	jive::output * state,
	size_t nstates)
{
	return jive_state_split(&dynamic_cast<const jive::state::type &>(*statetype), state, nstates);
}


#endif
