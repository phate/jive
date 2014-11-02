/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <fcntl.h>
#include <sys/mman.h>

#include <jive/util/buffer.h>

#include <unistd.h>

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
	
	if (ftruncate(fd, self->data.size()) == 0) {
		void * writable = mmap(0, self->data.size(), PROT_WRITE, MAP_SHARED, fd, 0);
		
		if (writable) {
			memcpy(writable, &self->data[0], self->data.size());
			munmap(writable, self->data.size());
			executable = mmap(0, self->data.size(), PROT_EXEC, MAP_SHARED, fd, 0);
		}
	}
	
	close(fd);
	return executable;
}
