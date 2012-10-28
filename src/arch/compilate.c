/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>

#include <fcntl.h>
#include <sys/mman.h>

void
jive_compilate_init(struct jive_compilate * self, struct jive_context * context)
{
	jive_buffer_init(&self->code_buffer, context);
	jive_buffer_init(&self->data_buffer, context);
	jive_buffer_init(&self->rodata_buffer, context);
	jive_buffer_init(&self->bss_buffer, context);
}

void
jive_compilate_fini(struct jive_compilate * self)
{
	jive_buffer_fini(&self->code_buffer);
	jive_buffer_fini(&self->data_buffer);
	jive_buffer_fini(&self->rodata_buffer);
	jive_buffer_fini(&self->bss_buffer);
}

void
jive_compilate_clear(jive_compilate * self)
{
	jive_buffer_resize(&self->code_buffer, 0);
	jive_buffer_resize(&self->data_buffer, 0);
	jive_buffer_resize(&self->rodata_buffer, 0);
	jive_buffer_resize(&self->bss_buffer, 0);
}

jive_buffer *
jive_compilate_get_buffer(struct jive_compilate * self, jive_stdsectionid section)
{
	switch(section) {
		case jive_stdsectionid_code:
			return &self->code_buffer;
		case jive_stdsectionid_data:
			return &self->data_buffer;
		case jive_stdsectionid_rodata:
			return &self->rodata_buffer;
		case jive_stdsectionid_bss:
			return &self->bss_buffer;
		default:
			return NULL; 
	}
}

void *
jive_compilate_map_to_memory(const jive_compilate * self)
{
	void * executable = NULL;
	
	size_t size = self->code_buffer.size + self->data_buffer.size + self->rodata_buffer.size
		+ self->bss_buffer.size;
	char template[] = "/tmp/jive-exec-buffer-XXXXXX";
#if defined(_GNU_SOURCE) && defined(O_CLOEXEC)
	int fd = mkostemp(template, O_CLOEXEC);
#else
	int fd = mkstemp(template);
	if (fd >= 0) fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
	if (fd < 0)
		return NULL;
	unlink(template);

	if (ftruncate(fd, size) == 0) {
		void * writable = mmap(0, size, PROT_WRITE, MAP_SHARED, fd, 0);

		if (writable) {
			size_t tmp = 0;
			memcpy(writable, self->code_buffer.data, self->code_buffer.size);
			tmp += self->code_buffer.size;
			memcpy(writable+tmp, self->data_buffer.data, self->data_buffer.size);
			tmp += self->data_buffer.size; 
			memcpy(writable+tmp, self->rodata_buffer.data, self->rodata_buffer.size);
			tmp += self->rodata_buffer.size;
			memcpy(writable+tmp, self->bss_buffer.data, self->bss_buffer.size);
			tmp += self->bss_buffer.size;
			munmap(writable, size);
			executable = mmap(0, size, PROT_EXEC, MAP_SHARED, fd, 0);
		}
	}

	close(fd);
	return executable;
}
