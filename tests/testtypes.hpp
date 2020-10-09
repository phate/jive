/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTTYPES_HPP
#define JIVE_TESTS_TESTTYPES_HPP

#include <jive/rvsdg/type.hpp>

namespace jive {
namespace test {

/* test value type */

class valuetype final : public jive::valuetype {
public:
	virtual
	~valuetype() noexcept;

	inline constexpr
	valuetype() noexcept
	: jive::valuetype()
	{}

	virtual
	std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
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
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;
};

}}

#endif
