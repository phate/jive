#ifndef JIVE_CONTEXT_H
#define JIVE_CONTEXT_H

/* size_t */
#include <unistd.h>

/**
	\file context.h
	
	\brief Memory allocator implementation
	
	Implements a fast allocator that reserves memory from a growing pool.
	It does not support reclamation of individual blocks of reserved memory
	at all, instead the memory is retained until the complete pool is
	freed.
*/

/**
	\brief Allocator context
	
	See: \ref jive_context_create
	\ref jive_context_destroy
	\ref jive_context_malloc
*/
typedef struct jive_context jive_context;

/**
	\brief Fatal error handler
	
	See: \ref jive_set_fatal_error_handler
*/
typedef void (*jive_error_f)(void * user_context, const char * msg);

/**
	\brief Create a compiler context
	\return Newly created context
*/
jive_context*
jive_context_create(void);

/**
	\brief Set handler for fatal errors
	
	\param ctx Compiler context
	\param function Function to be called on fatal error
	\param user_context First argument passed to called function
	\return A newly-created context, or NULL if insufficient memory was available
	
	Installs a function that will be called on encountering a fatal error.
	This function <B>must not</B> return to the caller, instead it
	should either abort the program or longjmp out of whatever
	operation was in progress.
*/
void
jive_set_fatal_error_handler(jive_context * ctx, jive_error_f function, void * user_context);

/**
	\brief Allocate memory within compiler context
	\param ctx Compiler context
	\param size Number of bytes to allocate
	\returns Block of allocated memory
	
	The function will return an allocated block of memory.
	If no memory could be allocated, the fatal error handler
	associated with the context will be invoked.
*/
void*
jive_context_malloc(jive_context *ctx, size_t size);

/**
	\brief Destroy compiler context
	\param ctx Compiler context
	
	Destroys the given compiler context, freeing all memory
	associated with the context.
*/
void
jive_context_destroy(jive_context *ctx);

/**
	\brief Notify context of a fatal error
	\param ctx Compiler context
	\param msg Error message
	
	Invokes the fatal error handler. This function will
	not return to the caller.
*/
void
jive_context_fatal_error(jive_context *ctx, const char *msg);

#endif
