/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDTYPE_H
#define JIVE_TYPES_RECORD_RCDTYPE_H

#include <jive/common.h>
#include <jive/vsdg/type.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace jive {
namespace rcd {

/* declaration */

class declaration final {
public:
	inline
	declaration()
	{}

	inline
	declaration(const std::vector<const valuetype*> & types)
	: types_(types.size())
	{
		std::transform(types.begin(), types.end(), types_.begin(),
			[](const auto & t){ return t->copy(); });
	}

	declaration(const declaration & other) = delete;

	declaration &
	operator=(const declaration & other) = delete;

	inline size_t
	nelements() const noexcept
	{
		return types_.size();
	}

	const valuetype &
	element(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nelements());
		return *static_cast<const valuetype*>(types_[index].get());
	}

	void
	append(const jive::valuetype & type)
	{
		types_.push_back(type.copy());
	}

private:
	std::vector<std::unique_ptr<jive::type>> types_;
};

/* type */

class type final : public jive::valuetype {
public:
	virtual ~type() noexcept;

	inline
	type(std::shared_ptr<const rcd::declaration> decl) noexcept
		: decl_(std::move(decl))
	{
	}

	inline const std::shared_ptr<const rcd::declaration> &
	declaration() const noexcept
	{
		return decl_;
	}

	virtual std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & type) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

private:
	std::shared_ptr<const rcd::declaration> decl_;
};

}
}

#endif
