/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/types/function/fcttype.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale( LC_ALL, "" ) ;

	jive::bits::type btype0(8);
	jive::bits::type btype1(8);
	const jive::base::type*  tmparray0[] = { &btype0 };
	const jive::base::type*  tmparray1[] = { &btype0 };

	jive::fct::type type0(1, tmparray0, 1, tmparray1);

	const jive::base::type*  tmparray2[] = { &btype0 };
	const jive::base::type*  tmparray3[] = { &btype1 };
	jive::fct::type type1(1, tmparray2, 1, tmparray3);

	const jive::base::type*  tmparray4[] = { &btype0 };
	const jive::base::type*  tmparray5[] = { &btype1, &btype1 };
	jive::fct::type type2(1, tmparray4, 2, tmparray5);

	const jive::base::type*  tmparray6[] = { &btype0, &btype0 };
	const jive::base::type*  tmparray7[] = { &btype0 };
	jive::fct::type type3(2, tmparray6, 1, tmparray7);

	assert(type0 == type0);
	assert(type0 == type1);
	assert(type0 != type2);
	assert(type0 != type3);
	
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-equals", test_main);
