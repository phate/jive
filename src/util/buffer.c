/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <fcntl.h>
#include <sys/mman.h>

#include <jive/util/buffer.h>

namespace jive {

jive::buffer &
buffer::append(const void * data, size_t nbytes)
{
	const uint8_t * d = static_cast<const uint8_t*>(data);
	for (size_t n = 0; n < nbytes; n++)
		data_.push_back(d[n]);

	return *this;
}

}

void *
jive_buffer_executable(const jive_buffer * self)
{
	void * executable = NULL;
	
	char filename_template[] = "/tmp/jive-exec-buffer-XXXXXX";
#if defined(_GNU_SOURCE) && defined(O_CLOEXEC)
	int fd = mkostemp(filename_template, O_CLOEXEC);
#else
	int fd = mkstemp(filename_template);
	if (fd >= 0) fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
	if (fd < 0)
		return 0;
	unlink(filename_template);
	
	if (ftruncate(fd, self->size) == 0) {
		void * writable = mmap(0, self->size, PROT_WRITE, MAP_SHARED, fd, 0);
		
		if (writable) {
			memcpy(writable, self->data, self->size);
			munmap(writable, self->size);
			executable = mmap(0, self->size, PROT_EXEC, MAP_SHARED, fd, 0);
		}
	}
	
	close(fd);
	return executable;
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

