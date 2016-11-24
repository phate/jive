/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SUBSTITUTION_H
#define JIVE_VSDG_SUBSTITUTION_H

#include <jive/common.h>

#include <unordered_map>

namespace jive {

class gate;

class substitution_map final {
public:
	inline jive::oport *
	lookup(const jive::oport * original) const noexcept
	{
		auto i = output_map_.find(original);
		return i != output_map_.end() ? i->second : nullptr;
	}

	inline jive::region *
	lookup(const jive::region * original) const noexcept
	{
		auto i = region_map_.find(original);
		return i != region_map_.end() ? i->second : nullptr;
	}

	inline jive::gate *
	lookup(const jive::gate * original) const noexcept
	{
		auto i = gate_map_.find(original);
		return i != gate_map_.end() ? i->second : nullptr;
	}

	inline void
	insert(const jive::output * original, jive::output * substitute)
	{
		output_map_[original] = substitute;
	}

	inline void
	insert(const jive::region * original, jive::region * substitute)
	{
		region_map_[original] = substitute;
	}
	inline void
	insert(const jive::gate * original, jive::gate * substitute)
	{
		gate_map_[original] = substitute;
	}

private:
	std::unordered_map<const jive::gate*, jive::gate*> gate_map_;
	std::unordered_map<const jive::region*, jive::region*> region_map_;
	std::unordered_map<const jive::oport*, jive::oport*> output_map_;
};

}

#endif
