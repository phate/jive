#ifndef JIVE_DEBUG_PRIVATE_H
#define JIVE_DEBUG_PRIVATE_H

#include <stdlib.h>
#include <stdio.h>

#define DEBUG 1

#define DEBUG_ASSERT(expr) \
	do {\
		if (!(expr)) {\
			fprintf(stderr, "%s (%s:%d): Assertion '%s' failed\n", __FUNCTION__, __FILE__, __LINE__, #expr );\
			abort();\
		}\
	} while(0)

#define DEBUG_PRINTF(fmt...) \
	do {\
		fprintf(stderr, fmt); \
	} while(0)


#endif
