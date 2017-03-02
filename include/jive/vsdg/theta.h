/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_THETA_H
#define JIVE_VSDG_THETA_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/operators/structural.h>

namespace jive {
class gate;
}

struct jive_theta_build_state;

namespace jive {

class theta_op final : public structural_op {
public:
	virtual
	~theta_op() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class theta_builder;

class loopvar final {
	friend theta_builder;

private:
	inline constexpr
	loopvar(jive::oport * value)
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

class theta_builder final {
public:
	inline
	theta_builder() noexcept
	: node_(nullptr)
	{}

	inline jive::region *
	region() const noexcept
	{
		return node_ ? node_->subregion(0) : nullptr;
	}

	inline jive::region *
	begin(jive::region * parent)
	{
		if (node_)
			return node_->subregion(0);

		node_ = parent->add_structural_node(jive::theta_op(), 1);
		return node_->subregion(0);
	}

	inline std::shared_ptr<jive::loopvar>
	add_loopvar(jive::oport * origin)
	{
		if (!node_)
			return nullptr;

		auto input = node_->add_input(&origin->type(), origin);
		auto argument = node_->subregion(0)->add_argument(input, origin->type());
		loopvars_.push_back(std::shared_ptr<loopvar>(new loopvar(loopvar(argument))));
		return loopvars_[loopvars_.size()-1];
	}

	inline jive::structural_node *
	end(jive::oport * predicate)
	{
		if (!node_)
			return nullptr;

		node_->subregion(0)->add_result(predicate, nullptr, jive::ctl::type(2));
		for (const auto & lv : loopvars_) {
			auto output = node_->add_output(&lv->value()->type());
			node_->subregion(0)->add_result(lv->value(), output, lv->value()->type());
			lv->set_value(output);
		}

		auto node = node_;
		node_ = nullptr;
		loopvars_.clear();

		return node;
	}

private:
	jive::structural_node * node_;
	std::vector<std::shared_ptr<loopvar>> loopvars_;
};

}

typedef struct jive_theta jive_theta;
typedef struct jive_theta_loopvar jive_theta_loopvar;

/**
	\brief Represent a theta construct under construction
*/
struct jive_theta {
	struct jive::region * region;
	struct jive_theta_build_state * internal_state;
};

/**
	\brief Represent information about a loop-variant value
*/
struct jive_theta_loopvar {
	jive::oport * value;
	jive::gate * gate;
};

/**
	\brief Begin constructing a loop region
*/
jive_theta
jive_theta_begin(struct jive::region * parent);

/**
	\brief Add a loop-variant variable with a pre-loop value
*/
jive_theta_loopvar
jive_theta_loopvar_enter(jive_theta self, jive::oport * pre_value);

/**
	\brief Set post-iteration value of a loop-variant variable
*/
void
jive_theta_loopvar_leave(jive_theta self, jive::gate * var, jive::oport * post_value);

/**
	\brief End constructing a loop region, specify repetition predicate
*/
jive::node *
jive_theta_end(jive_theta self, jive::oport * predicate,
	size_t npost_values, jive_theta_loopvar * post_values);

#endif
