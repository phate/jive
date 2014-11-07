/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/common.h>
#include <jive/util/intrusive-hash.h>

namespace {

class unit_test {
public:
	typedef int (*function_type)(void);
private:
	std::string name_;
	jive::detail::intrusive_hash_anchor<unit_test> hash_anchor_;
	function_type function_;
public:
	typedef jive::detail::intrusive_hash_accessor<
		std::string,
		unit_test,
		&unit_test::name_,
		&unit_test::hash_anchor_> hash_accessor;

	inline unit_test(
		std::string name,
		function_type function)
		: name_(std::move(name))
		, function_(function)
	{
	}

	inline int
	run() const
	{
		return function_();
	}
};

typedef jive::detail::owner_intrusive_hash<
	std::string, unit_test, unit_test::hash_accessor> unit_test_map;

static unit_test_map unit_tests;

}

void
jive_unit_test_register(const char * name, int (*fn)(void))
{
	std::unique_ptr<unit_test> test(new unit_test(name, fn));
	unit_tests.insert(std::move(test));
}

int
jive_unit_test_run(const char * name)
{
	auto i = unit_tests.find(name);
	JIVE_ASSERT(i != unit_tests.end());
	return i->run();
}
