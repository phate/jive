/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDGROUP_H
#define JIVE_TYPES_RECORD_RCDGROUP_H

#include <jive/types/record/rcdtype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/simple.h>

namespace jive {
namespace rcd {

class group_op final : public jive::simple_op {
public:
	virtual
	~group_op() noexcept;

	inline
	group_op(std::shared_ptr<const rcd::declaration> & declaration) noexcept
	: result_(jive::rcd::type(declaration))
	{
		for (size_t n = 0; n < declaration->nelements(); n++)
			arguments_.push_back({declaration->element(n)});
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

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

	inline const std::shared_ptr<const rcd::declaration> &
	declaration() const noexcept
	{
		return static_cast<const jive::rcd::type*>(&result_.type())->declaration();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port result_;
	std::vector<jive::port> arguments_;
};

}
}

jive::output *
jive_group_create(std::shared_ptr<const jive::rcd::declaration> & decl,
	size_t narguments, jive::output * const * arguments);

jive::output *
jive_empty_group_create(jive::graph * graph, std::shared_ptr<const jive::rcd::declaration> & decl);

#endif
