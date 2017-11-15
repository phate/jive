/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_TYPE_H
#define JIVE_ARCH_IMMEDIATE_TYPE_H

#include <jive/vsdg/type.h>

namespace jive {
namespace imm {

class type final : public jive::valuetype {
public:
	virtual ~type() noexcept;

	inline constexpr
	type() noexcept
	: jive::valuetype()
	{}

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual std::unique_ptr<base::type>
	copy() const override;
};

}
}

#endif
