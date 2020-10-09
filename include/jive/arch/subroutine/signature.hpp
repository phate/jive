/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_SIGNATURE_HPP
#define JIVE_ARCH_SUBROUTINE_SIGNATURE_HPP

#include <string>
#include <vector>

#include <jive/arch/subroutine/nodes.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/node.hpp>

namespace jive {

class output;
class subroutine_abi;

class subroutine_machine_signature {
public:
	inline subroutine_machine_signature() noexcept
		: stack_frame_lower_bound(0)
		, stack_frame_upper_bound(0)
		, use_frame_pointer(false)
	{
	}

	struct passthrough {
		std::string name;
		const jive::resource_class * rescls;
		bool may_spill;
	};
	struct argument {
		std::string name;
		const jive::resource_class * rescls;
		bool may_spill;
	};
	struct result {
		std::string name;
		const jive::resource_class * rescls;
	};

	std::vector<passthrough> passthroughs;
	std::vector<argument> arguments;
	std::vector<result> results;

	ssize_t stack_frame_lower_bound;
	ssize_t stack_frame_upper_bound;
	bool use_frame_pointer;
	const jive::subroutine_abi * abi_class;
};

}

#endif
