/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jive/common.h>
#include <jive/context.h>
#include <jive/util/list.h>

#include "compiler.h"
#include "platform-backtrace.h"

typedef struct jive_context_block jive_context_block;

struct jive_context_block {
	struct {
		jive_context_block * prev;
		jive_context_block * next;
	} context_block_list;
#ifdef JIVE_MALLOC_TRACE
	jive_platform_backtrace trace;
	size_t size;
#endif
};

/**
	\internal
	\brief Allocator data structure
*/
struct jive_context {
	/** area from which allocations are served currently */
	struct {
		jive_context_block * first;
		jive_context_block * last;
	} blocks;
	
	/** error handling */
	struct {
		jive_error_f function;
		void * user_context;
	} error;
};

jive_context *
jive_context_create(void)
{
	jive_context * ctx;
	
	ctx = (jive_context *)malloc(sizeof(*ctx));
	if (!ctx) return 0;
	
	ctx->blocks.first = ctx->blocks.last = 0;
	
	ctx->error.function=0;
	ctx->error.user_context=0;
	
	return ctx;
}

void
jive_set_fatal_error_handler(jive_context * ctx, jive_error_f function, void * user_context)
{
	ctx->error.function=function;
	ctx->error.user_context=user_context;
}

void *
jive_context_malloc(jive_context * ctx, size_t size)
{
	jive_context_block * block;
	
	block = malloc(sizeof(*block) + size);
	
	if ( unlikely(!block) ) {
		jive_context_fatal_error(ctx, "Failed to allocate memory");
		/* never reached, but silences compiler */
		return 0;
	}
	
#ifdef JIVE_MALLOC_TRACE
	jive_platform_backtrace_collect(&block->trace);
	block->size = size;
#endif
	
	JIVE_LIST_PUSH_BACK(ctx->blocks, block, context_block_list);
	
	return block + 1;
}

void
jive_context_free(jive_context * ctx, void * ptr)
{
	if ( unlikely(ptr == 0) ) return;
	jive_context_block * block = ((jive_context_block *) ptr) - 1;
	JIVE_LIST_REMOVE(ctx->blocks, block, context_block_list);
	free(block);
}

void *
jive_context_realloc(jive_context * ctx, void * ptr, size_t new_size)
{
	jive_context_block * orig_block = 0;
	if ( likely(ptr != 0) ) {
		orig_block = ((jive_context_block *) ptr) - 1;
		JIVE_LIST_REMOVE(ctx->blocks, orig_block, context_block_list);
	}
	jive_context_block * new_block = realloc(orig_block, sizeof(*new_block) + new_size);
	if ( unlikely(!new_block) ) {
		if (orig_block) JIVE_LIST_PUSH_BACK(ctx->blocks, orig_block, context_block_list);
		jive_context_fatal_error(ctx, "Failed to allocate memory");
		/* never reached, but silences compiler */
		return 0;
	}
	JIVE_LIST_PUSH_BACK(ctx->blocks, new_block, context_block_list);
	
#ifdef JIVE_MALLOC_TRACE
	jive_platform_backtrace_collect(&new_block->trace);
	new_block->size = new_size;
#endif
	
	return new_block + 1;
}

void
jive_context_assert_clean(jive_context * ctx)
{
	if (!ctx->blocks.first)
		return;
	
#ifdef JIVE_MALLOC_TRACE
	jive_context_block * block;
	JIVE_LIST_ITERATE(ctx->blocks, block, context_block_list) {
		char tmp[128];
		ssize_t count = snprintf(tmp, sizeof(tmp), "Block size %zd at %p allocated by\n",
			block->size, block + 1);
		write(2, tmp, count);
		jive_platform_backtrace_print(&block->trace);
		write(2, "\n", 1);
	}
#endif
	
	jive_context_fatal_error(ctx, "Unreclaimed memory in context");
}

void
jive_context_destroy(jive_context * ctx)
{
	while(ctx->blocks.first) {
		jive_context_block * block = ctx->blocks.first;
		JIVE_LIST_REMOVE(ctx->blocks, block, context_block_list);
		free(block);
	}
	
	free(ctx);
}

void
jive_context_fatal_error(jive_context * ctx, const char * msg)
{
	if (!ctx->error.function) {
		fprintf(stderr, "%s\n", msg);
		abort();
	} else {
		ctx->error.function(ctx->error.user_context, msg);
		abort();
	}
}

bool
jive_context_is_empty(jive_context * context)
{
	return context->blocks.first == 0;
}

/* string helpers, don't really belong here */

static char *
jive_strcat_helper(jive_context * ctx, char * str, const char * static_append)
{
	size_t len = 0;
	if (str) len = strlen(str);
	size_t append_len = 0;
	if (static_append) append_len = strlen(static_append);
	size_t total = len + append_len;
	
	str = jive_context_realloc(ctx, str, total + 1);
	memcpy(str + len, static_append, append_len + 1);
	
	return str;
}

char *
jive_context_strjoin(jive_context * context, ...)
{
	va_list args;
	
	char * result = 0, *s;
	
	va_start(args, context);
	
	s = va_arg(args, char *);
	while(s) {
		result = jive_strcat_helper(context, result, s);
		s = va_arg(args, char *);
	}
	va_end(args);
	
	return result;
}

char *
jive_context_strdup(jive_context * context, const char * str)
{
	size_t len = strlen(str);
	char * s = jive_context_malloc(context, len + 1);
	memcpy(s, str, len+1);
	return s;
}
