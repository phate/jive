/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTTYPE_H
#define JIVE_TYPES_FLOAT_FLTTYPE_H

#include <jive/rvsdg/type.h>

namespace jive {
namespace flt {

/* float type */

class type final : public jive::valuetype {
public:
	virtual ~type() noexcept;

	inline constexpr
	type() noexcept
	: jive::valuetype()
	{}

	virtual std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;
};

}
}

#endif
