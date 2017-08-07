/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_BASE_H
#define JIVE_VSDG_OPERATORS_BASE_H

#include <memory>
#include <string>
#include <vector>

class jive_resource_class;

namespace jive {

namespace base {
class type;
}

class node;
class node_normal_form;
class output;
class region;

class operation {
public:
	virtual ~operation() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept = 0;

	virtual size_t
	narguments() const noexcept = 0;
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept = 0;
	// FIXME: specific to register allocator, should go away
	virtual const jive_resource_class *
	argument_cls(size_t index) const noexcept;

	virtual size_t
	nresults() const noexcept = 0;
	virtual const jive::base::type &
	result_type(size_t index) const noexcept = 0;
	// FIXME: specific to register allocator, should go away
	virtual const jive_resource_class *
	result_cls(size_t index) const noexcept;

	virtual std::string
	debug_string() const = 0;

	virtual std::unique_ptr<jive::operation>
	copy() const = 0;

	inline bool
	operator!=(const operation & other) const noexcept
	{
		return ! (*this == other);
	}
};

}

#endif
