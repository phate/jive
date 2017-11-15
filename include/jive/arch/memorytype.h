/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMORYTYPE_H
#define JIVE_ARCH_MEMORYTYPE_H

#include <jive/vsdg/type.h>

namespace jive {
namespace mem {

class type final : public jive::statetype {
public:
	virtual ~type() noexcept;

	inline constexpr
	type() noexcept
	: jive::statetype()
	{}

	virtual std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

	inline static const type & instance() { return instance_; }

private:
	static const type instance_;
};

}
}

#endif
