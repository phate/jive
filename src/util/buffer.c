#include <sys/mman.h>

#include <jive/util/buffer.h>

void *
jive_buffer_executable(const jive_buffer * self)
{
	void * addr = mmap(0, self->size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (!addr) return 0;
	
	memcpy(addr, self->data, self->size);
	mprotect(addr, self->size, PROT_EXEC);
	return addr;
}

void *
jive_buffer_reserve_slow(jive_buffer * self, size_t size)
{
	size_t total = (self->size+size) * 2;
	void * tmp = jive_context_realloc(self->context, self->data, total);
	if (!tmp) return 0;
	self->available = total;
	self->data = tmp;
	
	tmp = self->size + (char *)(self->data);
	self->size += size;
	
	return tmp;
}

