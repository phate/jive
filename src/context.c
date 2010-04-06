#include <stdlib.h>
#include <stdio.h>
#include <jive/context.h>

#include "compiler.h"

/**
	\internal
	\brief Memory area used by allocator
*/
typedef struct allocator_area {
	/** pointer to base of area */
	void *base;
	/** total size of area */
	int size;
	/** number of bytes already used */
	int used;
	/** link to older memory area */
	struct allocator_area *next;
} allocator_area;

/**
	\internal
	\brief Allocator data structure
*/
struct jive_context {
	/** area from which allocations are served currently */
	allocator_area *area;
	
	/** error handling */
	struct {
		jive_error_f function;
		void *user_context;
	} error;
};

static allocator_area *allocator_area_create(int size)
{
	allocator_area *area;
	
	area=(allocator_area *)malloc(sizeof(*area));
	if (!area) return 0;
	
	area->base=malloc(size);
	if (!area->base) {
		free(area);
		return 0;
	}
	area->size=size;
	area->used=0;
	area->next=0;
	
	return area;
}

static void allocator_area_destroy(allocator_area *area)
{
	free(area->base);
	free(area);
}

jive_context *jive_context_create(void)
{
	jive_context * ctx;
	
	ctx=(jive_context *)malloc(sizeof(*ctx));
	if (!ctx) return 0;
	
	ctx->area=allocator_area_create(4096-16);
	if (!ctx->area) {
		free(ctx);
		return 0;
	}
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

void*
jive_context_malloc(jive_context *ctx, size_t size)
{
	/* align memory returned */
	size_t reserve=(size+(sizeof(long)-1))&~(sizeof(long)-1);
	
	if (unlikely(ctx->area->size - ctx->area->used < reserve)) {
		size_t newsize=(reserve+4095)&~4095;
		
		/* reserve at least twice the amount of memory as
		last time, to gradually reduce allocation overhead */
		if (newsize<2*ctx->area->size)
			newsize=2*ctx->area->size;
		
		allocator_area *area=allocator_area_create(newsize);
		if (!area) {
			jive_context_fatal_error(ctx, "Failed to allocate memory");
			/* never reached, but silence compiler */
			return 0;
		}
		area->next=ctx->area;
		ctx->area=area;
	}
	
	void *tmp=((char *)ctx->area->base) + ctx->area->used;
	ctx->area->used+=reserve;
	
	return tmp;
}

void
jive_context_destroy(jive_context *ctx)
{
	while(ctx->area) {
		allocator_area *next=ctx->area->next;
		allocator_area_destroy(ctx->area);
		ctx->area=next;
	}
	
	free(ctx);
}

void
jive_context_fatal_error(jive_context *ctx, const char *msg)
{
	if (!ctx->error.function) {
		fprintf(stderr, "%s\n", msg);
		abort();
	} else {
		ctx->error.function(ctx->error.user_context, msg);
		abort();
	}
}
