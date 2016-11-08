/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GAMMA_H
#define JIVE_VSDG_GAMMA_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>

struct jive_graph;

namespace jive {
	class output;
	class type;
}

namespace jive {

class gamma_head_op final : public region_head_op {
public:
	virtual
	~gamma_head_op() noexcept;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class gamma_tail_op final : public region_tail_op {
public:
	virtual
	~gamma_tail_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class gamma_op final : public region_anchor_op {
public:
	virtual
	~gamma_op() noexcept;

	inline
	gamma_op(size_t nalternatives) noexcept
		: predicate_type_(nalternatives)
	{
	}

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	inline size_t
	nalternatives() const noexcept
	{
		return predicate_type_.nalternatives();
	}

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	virtual bool
	operator==(const operation & other) const noexcept override;

private:
	jive::ctl::type predicate_type_;
};

}

std::vector<jive::output*>
jive_gamma(jive::output * predicate,
	const std::vector<const jive::base::type*> & types,
	const std::vector<std::vector<jive::output*>> & alternatives);

#endif
