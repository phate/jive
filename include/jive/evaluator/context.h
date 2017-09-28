/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_EVALUATOR_CONTEXT_H
#define JIVE_EVALUATOR_CONTEXT_H

#include <jive/common.h>
#include <jive/evaluator/literal.h>

#include <memory>
#include <unordered_map>

namespace jive {

class output;

namespace eval {

class frame {
public:
	inline
	~frame() noexcept
	{}

	inline
	frame() noexcept
	{}

	inline bool
	exists(const jive::output * output) const noexcept
	{
		return literals_.find(output) != literals_.end();
	}

	inline const literal *
	lookup(const jive::output * output) const noexcept
	{
		JIVE_DEBUG_ASSERT(exists(output));
		return literals_.find(output)->second.get();
	}

	void
	insert(const jive::output * output, const literal * v)
	{
		JIVE_DEBUG_ASSERT(!exists(output));
		JIVE_DEBUG_ASSERT(output->type() == v->type());
		literals_[output] = std::move(v->copy());
	}

private:
	std::unordered_map<const jive::output *, std::unique_ptr<const literal>> literals_;
};

class context final {
public:
	inline bool
	has_frames(struct jive::region * region) const noexcept
	{
		return frames_.find(region) != frames_.end();
	}

	inline size_t
	nframes(struct jive::region * region) const noexcept
	{
		JIVE_DEBUG_ASSERT(has_frames(region));
		return frames_.find(region)->second.size();
	}

	inline void
	push_frame(struct jive::region * region)
	{
		if (frames_.find(region) == frames_.end())
			frames_[region] = std::vector<std::unique_ptr<frame>>();
		frames_[region].emplace_back(std::unique_ptr<frame>(new frame()));
	}

	inline void
	pop_frame(struct jive::region * region)
	{
		JIVE_DEBUG_ASSERT(has_frames(region));
		JIVE_DEBUG_ASSERT(nframes(region));
		frames_[region].pop_back();
	}

	inline size_t
	narguments() const noexcept
	{
		return arguments_.size();
	}

	inline void
	push_arguments(const std::vector<const literal*> & arguments)
	{
		arguments_.push_back(std::vector<std::unique_ptr<const literal>>());
		for (size_t n = 0; n < arguments.size(); n++)
			arguments_[arguments_.size()-1].emplace_back(arguments[n]->copy());
	}

	inline void
	push_arguments(const std::vector<std::unique_ptr<const literal>> & arguments)
	{
		arguments_.push_back(std::vector<std::unique_ptr<const literal>>());
		for (size_t n = 0; n < arguments.size(); n++)
			arguments_[arguments_.size()-1].emplace_back(arguments[n]->copy());
	}

	inline const std::vector<std::unique_ptr<const literal>> &
	top_arguments() const
	{
		JIVE_DEBUG_ASSERT(arguments_.size() != 0);
		return arguments_.back();
	}

	inline void
	pop_arguments()
	{
		if (arguments_.size() == 0)
			return;
		arguments_.pop_back();
	}

	inline const std::vector<std::unique_ptr<const literal>>
	poptop_arguments()
	{
		std::vector<std::unique_ptr<const literal>> arguments;
		for (size_t n = 0; n < top_arguments().size(); n++)
			arguments.emplace_back(top_arguments()[n]->copy());
		pop_arguments();
		return arguments;
	}

	inline bool
	exists(const jive::input * input) const noexcept
	{
		return exists(input->origin());
	}

	inline bool
	exists(const jive::output * output) const noexcept
	{
		jive::region * region = output->region();

		if (frames_.find(region) == frames_.end())
			return false;

		if (frames_.find(region)->second.back()->exists(output))
			return true;

		return false;
	}

	inline const literal *
	lookup(const jive::input * input) const noexcept
	{
		return lookup(input->origin());
	}

	inline const literal *
	lookup(const jive::output * output) const noexcept
	{
		jive::region * region = output->region();

		if (frames_.find(region) == frames_.end())
			return nullptr;

		if (frames_.find(region)->second.back()->exists(output))
			return frames_.find(region)->second.back()->lookup(output);

		return nullptr;
	}

	void
	insert(const jive::output * output, const literal * v)
	{
		JIVE_DEBUG_ASSERT(!exists(output));
		frames_.find(output->region())->second.back()->insert(output, v);
	}

private:
	std::vector<std::vector<std::unique_ptr<const literal>>> arguments_;
	std::unordered_map<struct jive::region *, std::vector<std::unique_ptr<frame>>> frames_;
};

}
}

#endif
