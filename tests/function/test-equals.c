/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/types/bitstring.h>
#include <jive/types/function/fcttype.h>

static int test_main(void)
{
	setlocale( LC_ALL, "" ) ;

	jive_context* context = jive_context_create() ;

	jive_bitstring_type btype0(8);
	jive_bitstring_type btype1(8);
	const jive_type*  tmparray0[] = { &btype0 };
	const jive_type*  tmparray1[] = { &btype0 };

	jive_function_type type0(1, tmparray0, 1, tmparray1);

	const jive_type*  tmparray2[] = { &btype0 };
	const jive_type*  tmparray3[] = { &btype1 };
	jive_function_type type1(1, tmparray2, 1, tmparray3);

	const jive_type*  tmparray4[] = { &btype0 };
	const jive_type*  tmparray5[] = { &btype1, &btype1 };
	jive_function_type type2(1, tmparray4, 2, tmparray5);

	const jive_type*  tmparray6[] = { &btype0, &btype0 };
	const jive_type*  tmparray7[] = { &btype0 };
	jive_function_type type3(2, tmparray6, 1, tmparray7);

	assert( jive_type_equals( &type0, &type0 ) ) ;
	assert( jive_type_equals( &type0, &type1 ) ) ;
	assert( !jive_type_equals( &type0, &type2 ) ) ;
	assert( !jive_type_equals( &type0, &type3 ) ) ;
	
	assert( jive_context_is_empty(context) ) ;
	jive_context_destroy( context ) ;
	
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-equals", test_main);
