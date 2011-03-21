#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/bitstring.h>
#include <jive/vsdg/functiontype.h>

int main()
{
	setlocale( LC_ALL, "" ) ;

	jive_context* context = jive_context_create() ;

	JIVE_DECLARE_BITSTRING_TYPE( btype0, 8 ) ;
	JIVE_DECLARE_BITSTRING_TYPE( btype1, 8 ) ;

	jive_function_type* type0 = jive_function_type_create( context,
		1, (const jive_type* []) { btype0 },
		1, (const jive_type* []) { btype0 } ) ;
	jive_function_type* type1 = jive_function_type_create( context,
		1, (const jive_type* []) { btype0 },
		1, (const jive_type* []) { btype1 } ) ;
	jive_function_type* type2 = jive_function_type_create( context,
		1, (const jive_type* []) { btype0 },
		2, (const jive_type* []) { btype1, btype1 } ) ;
	jive_function_type* type3 = jive_function_type_create( context,
		2, (const jive_type* []) { btype0, btype0 },
		1, (const jive_type* []) { btype0 } ) ;

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
