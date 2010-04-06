#include <sys/mman.h>

#include <jive/buffer.h>

void *
jive_buffer_executable(const jive_buffer * buffer)
{
	void * addr = mmap(0, buffer->size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (!addr) return 0;
	
	memcpy(addr, buffer->data, buffer->size);
	mprotect(addr, buffer->size, PROT_EXEC);
	return addr;
}

void *
jive_buffer_reserve_slow(jive_buffer * buffer, size_t size)
{
	size_t total = (buffer->size+size) * 2;
	void * tmp = realloc(buffer->data, total);
	if (!tmp) return 0;
	buffer->available = total;
	buffer->data = tmp;
	
	tmp = buffer->size + (char *)(buffer->data);
	buffer->size += size;
	
	return tmp;
}

