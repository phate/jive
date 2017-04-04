/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_PHI_H
#define JIVE_VSDG_PHI_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/structural.h>
#include <jive/vsdg/structural_node.h>

/* phi node */

namespace jive {

class phi_op final : public structural_op {
public:
	virtual
	~phi_op() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}

JIVE_EXPORTED_INLINE jive::region *
jive_phi_region_cast(jive::region * region)
{
	/*
		FIXME: remove this function
	*/
	if (region->node() && typeid(region->node()->operation()) == typeid(jive::phi_op))
		return region;

	return nullptr;
}

JIVE_EXPORTED_INLINE const jive::region *
jive_phi_region_const_cast(const jive::region * region)
{
	/*
		FIXME: remove this function
	*/
	if (region->node() && typeid(region->node()->operation()) == typeid(jive::phi_op))
		return region;

	return nullptr;
}

namespace jive {

class phi_builder;

class recvar final {
	friend phi_builder;

private:
	inline constexpr
	recvar(jive::oport * value)
	: value_(value)
	{}

public:
	inline void
	set_value(jive::oport * value)
	{
		value_ = value;
	}

	inline jive::oport *
	value() const noexcept
	{
		return value_;
	}

private:
	jive::oport * value_;
};

class phi_builder final {
public:
	inline
	phi_builder() noexcept
	: node_(nullptr)
	{}

	inline jive::region *
	begin(jive::region * parent)
	{
		if (node_)
			node_->subregion(0);

		node_ = parent->add_structural_node(jive::phi_op(), 1);
		return node_->subregion(0);
	}

	inline jive::region *
	region() const noexcept
	{
		return node_ ? node_->subregion(0) : nullptr;
	}

	inline jive::oport *
	add_dependency(jive::oport * value)
	{
		if (!node_)
			return nullptr;

		auto input = node_->add_input(&value->type(), value);
		return node_->subregion(0)->add_argument(input, value->type());
	}

	inline std::shared_ptr<jive::recvar>
	add_recvar(const jive::base::type & type)
	{
		if (!node_)
			return nullptr;

		auto argument = node_->subregion(0)->add_argument(nullptr, type);
		recvars_.push_back(std::shared_ptr<recvar>(new recvar(argument)));
		return recvars_[recvars_.size()-1];
	}

	inline jive::structural_node *
	end()
	{
		if (!node_)
			return nullptr;

		for (const auto & rv : recvars_) {
			auto output = node_->add_output(&rv->value()->type());
			node_->subregion(0)->add_result(rv->value(), output, rv->value()->type());
			rv->set_value(output);
		}

		auto node = node_;
		node_ = nullptr;
		recvars_.clear();

		return node;
	}

private:
	jive::structural_node * node_;
	std::vector<std::shared_ptr<recvar>> recvars_;
};

}

typedef struct jive_phi jive_phi;
typedef struct jive_phi_fixvar jive_phi_fixvar;

struct jive_phi_build_state;

/**
	\brief Represent a phi construct under construction
*/
struct jive_phi {
	struct jive::region * region;
	struct jive_phi_build_state * internal_state;
};

/**
	\brief Represent information about a phi fixpoint value
*/
struct jive_phi_fixvar {
	jive::oport * value;
	jive::gate * gate;
};

/**
	\brief Begin constructing a phi region
*/
jive_phi
jive_phi_begin(struct jive::region * parent);

/**
	\brief Add a fixpoint variable of given type
*/
jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive::base::type * type);

/**
	\brief Set fixpoint value of variable
*/
void
jive_phi_fixvar_leave(jive_phi self, jive::gate * var, jive::oport * post_value);

/**
	\brief End constructing a phi region
*/
jive::node *
jive_phi_end(jive_phi self,
	     size_t npost_values, jive_phi_fixvar * fix_values);

#endif
