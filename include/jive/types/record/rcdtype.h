/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDTYPE_H
#define JIVE_TYPES_RECORD_RCDTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace rcd {

/* declaration */

struct declaration {
	size_t nelements;
	const jive::value::type ** elements;
};

/* type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr
	type(const jive::rcd::declaration * decl) noexcept
		: decl_(decl)
	{
	}

	inline const jive::rcd::declaration * declaration() const noexcept { return decl_; }

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & type) const noexcept override;

	virtual jive::rcd::type * copy() const override;

private:
	const jive::rcd::declaration * decl_;
};

}
}

#endif
