/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDTYPE_H
#define JIVE_TYPES_RECORD_RCDTYPE_H

#include <jive/common.h>
#include <jive/vsdg/basetype.h>

#include <memory>
#include <vector>

namespace jive {
namespace rcd {

/* declaration */

class declaration final {
public:
	inline
	declaration(const std::vector<const value::type*> & types)
	{
		for (auto type : types)
			types_.emplace_back(std::unique_ptr<value::type>(type->copy()));
	}

	declaration(const declaration & other) = delete;

	declaration &
	operator=(const declaration & other) = delete;

	inline size_t
	nelements() const noexcept
	{
		return types_.size();
	}

	const value::type &
	element(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nelements());
		return *types_[index];
	}

private:
	std::vector<std::unique_ptr<value::type>> types_;
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
