/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
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

	JIVE_DECLARE_BITSTRING_TYPE( btype0, 8 ) ;
	JIVE_DECLARE_BITSTRING_TYPE( btype1, 8 ) ;
const jive_type*  tmparray0[] = { btype0 };
const jive_type*  tmparray1[] = { btype0 };

	jive_function_type* type0 = jive_function_type_create( context,
		1, tmparray0,
		1, tmparray1 ) ;
	const jive_type*  tmparray2[] = { btype0 };
	const jive_type*  tmparray3[] = { btype1 };
	jive_function_type* type1 = jive_function_type_create( context,
		1, tmparray2,
		1, tmparray3 ) ;
	const jive_type*  tmparray4[] = { btype0 };
	const jive_type*  tmparray5[] = { btype1, btype1 };
	jive_function_type* type2 = jive_function_type_create( context,
		1, tmparray4,
		2, tmparray5 ) ;
	const jive_type*  tmparray6[] = { btype0, btype0 };
	const jive_type*  tmparray7[] = { btype0 };
	jive_function_type* type3 = jive_function_type_create( context,
		2, tmparray6,
		1, tmparray7 ) ;

	assert( jive_type_equals( &type0->base.base, &type0->base.base ) ) ;
	assert( jive_type_equals( &type0->base.base, &type1->base.base ) ) ;
	assert( !jive_type_equals( &type0->base.base, &type2->base.base ) ) ;
	assert( !jive_type_equals( &type0->base.base, &type3->base.base ) ) ;
	
	jive_function_type_destroy( type0 ) ;
	jive_function_type_destroy( type1 ) ;
	jive_function_type_destroy( type2 ) ;
	jive_function_type_destroy( type3 ) ;
	
	assert( jive_context_is_empty(context) ) ;
	jive_context_destroy( context ) ;
	
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-equals", test_main);
