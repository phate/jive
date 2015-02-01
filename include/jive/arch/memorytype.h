/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/statetype.h>

namespace jive {
namespace mem {

class type final : public jive::state::type {
public:
	virtual ~type() noexcept;

	inline constexpr type() noexcept : jive::state::type() {};

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::mem::type * copy() const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;
};

class gate final : public jive::state::gate {
public:
	virtual ~gate() noexcept;

	gate(jive_graph * graph, const char name[]);

	virtual const jive::mem::type & type() const noexcept { return type_; }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::mem::type type_;
};

}
}

#endif
