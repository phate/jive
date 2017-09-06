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
	loopvar(
		jive::structural_input * input,
		jive::structural_output * output)
	: input_(input)
	, output_(output)
	{}

public:
	inline jive::structural_input *
	input() const noexcept
	{
		return input_;
	}

	inline jive::structural_output *
	output() const noexcept
	{
		return output_;
	}

	inline jive::argument *
	argument() const noexcept
	{
		if (input_ == nullptr)
			return nullptr;

		JIVE_DEBUG_ASSERT(input_->arguments.first != nullptr
		&& input_->arguments.first == input_->arguments.last);
		return input_->arguments.first;
	}

	inline jive::result *
	result() const noexcept
	{
		if (output_ == nullptr)
			return nullptr;

		JIVE_DEBUG_ASSERT(output_->results.first != nullptr
		&& output_->results.first == output_->results.last);
		return output_->results.first;
	}

private:
	jive::structural_input * input_;
	jive::structural_output * output_;
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
	add_loopvar(jive::output * origin)
	{
		if (!node_)
			return nullptr;

		auto input = node_->add_input(origin->type(), origin);
		node_->subregion(0)->add_argument(input, origin->type());
		loopvars_.push_back(std::make_shared<loopvar>(loopvar(input, nullptr)));
		return loopvars_.back();
	}

	inline jive::structural_node *
	end(
		jive::output * predicate,
		const std::unordered_map<std::shared_ptr<loopvar>, jive::output*> & lvmap)
	{
		if (!node_)
			return nullptr;

		node_->subregion(0)->add_result(predicate, nullptr, jive::ctl::type(2));
		for (const auto & lv : loopvars_) {
			auto it = lvmap.find(lv);
			auto value = it != lvmap.end() ? it->second : lv->argument();
			auto output = node_->add_output(lv->input()->type());
			node_->subregion(0)->add_result(value, output, lv->input()->type());
			lv->output_ = output;
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

#endif
