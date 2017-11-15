/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTTYPES_H
#define JIVE_TESTS_TESTTYPES_H

#include <jive/vsdg/type.h>

namespace jive {
namespace test {

/* test value type */

class valuetype final : public jive::value::type {
public:
	virtual
	~valuetype() noexcept;

	inline constexpr
	valuetype() noexcept
	: jive::value::type()
	{}

	virtual
	std::string debug_string() const override;

	virtual bool
	operator==(const jive::base::type & other) const noexcept override;

	virtual std::unique_ptr<base::type>
	copy() const override;
};

/* test state type */

class statetype final : public jive::statetype {
public:
	virtual
	~statetype() noexcept;

	inline constexpr
	statetype() noexcept
	: jive::statetype()
	{}

	virtual
	std::string debug_string() const override;

	virtual bool
	operator==(const jive::base::type & other) const noexcept override;

	virtual std::unique_ptr<base::type>
	copy() const override;
};

}}

#endif
