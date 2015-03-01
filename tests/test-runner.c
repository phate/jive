/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <iostream>

#include "test-registry.h"

int main(int argc, char ** argv)
{
	if (argc > 1) {
		return jive_unit_test_run(argv[1]);
	} else {
		for (const std::string & name : list_unit_tests()) {
			std::cout << name << "\n";
		}
	}
}
