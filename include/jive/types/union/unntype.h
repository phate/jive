/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNTYPE_H
#define JIVE_TYPES_UNION_UNNTYPE_H

#include <jive/vsdg/type.h>

namespace jive {
namespace unn {

/* declaration */

struct declaration {
	size_t nelements;
	const jive::valuetype ** elements;
};

/* type */

class type final : public jive::valuetype {
public:
	virtual ~type() noexcept;

	inline constexpr
	type(const jive::unn::declaration * decl) noexcept
		: decl_(decl)
	{
	}

	inline const jive::unn::declaration * declaration() const noexcept { return decl_; }

	virtual std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

private:
	const jive::unn::declaration * decl_;
};

}
}

#endif
