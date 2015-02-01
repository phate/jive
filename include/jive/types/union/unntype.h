/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNTYPE_H
#define JIVE_TYPES_UNION_UNNTYPE_H

#include <jive/vsdg/valuetype.h>

namespace jive {
namespace unn {

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
	type(const jive::unn::declaration * decl) noexcept
		: decl_(decl)
	{
	}

	inline const jive::unn::declaration * declaration() const noexcept { return decl_; }

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::unn::type * copy() const override;

private:
	const jive::unn::declaration * decl_;
};

/* gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(const jive::unn::declaration * decl, jive_graph * graph, const char name[]);

	inline const jive::unn::declaration *
	declaration() const noexcept
	{
		return static_cast<const jive::unn::type*>(&type())->declaration();
	}

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::unn::type type_;
};

}
}

#endif
