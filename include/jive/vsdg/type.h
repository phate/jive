/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_TYPE_H
#define JIVE_VSDG_TYPE_H

#include <memory>
#include <string>

namespace jive {
namespace base {

class type {
public:
	virtual
	~type() noexcept;

protected:
	inline constexpr
	type() noexcept
	{}

public:
	virtual bool
	operator==(const jive::base::type & other) const noexcept = 0;

	inline bool
	operator!=(const jive::base::type & other) const noexcept
	{
		return !(*this == other);
	}

	virtual std::unique_ptr<type>
	copy() const = 0;

	virtual std::string
	debug_string() const = 0;
};

}	//base namespace

class valuetype : public jive::base::type {
public:
	virtual
	~valuetype() noexcept;

protected:
	inline constexpr
	valuetype() noexcept
		: jive::base::type()
	{}
};

class statetype : public jive::base::type {
public:
	virtual
	~statetype() noexcept;

protected:
	inline constexpr
	statetype() noexcept
		: jive::base::type()
	{}
};

}

#endif
