#include <assert.h>
#include <setjmp.h>
#include <jive/context.h>

static void jump(void * where, const char * msg)
{
	jmp_buf * buffer=(jmp_buf *)where;
	longjmp(*buffer, 1);
}

int main(int argc, char **argv)
{
	jive_context * ctx=jive_context_create();
	
	void * volatile ptr=0;
	int error_handler_called=0;
	
	jmp_buf buffer;
	
	if (setjmp(buffer)==0) {
		jive_set_fatal_error_handler(ctx, jump, &buffer);
		ptr = jive_context_malloc(ctx, 1048576);
		jive_context_fatal_error(ctx, "Error");
	} else {
		error_handler_called=1;
	}
	
	assert(error_handler_called == 1);
	assert(ptr != 0);
	
	jive_context_destroy(ctx);
	
	return 0;
}
