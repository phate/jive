/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>

#include <fcntl.h>
#include <sys/mman.h>

static void jive_section_init(jive_section * self, jive_context * context,
	jive_stdsectionid sectionid)
{
	self->id = sectionid;
	jive_buffer_init(&self->contents, context);
}

static void jive_section_clear(jive_section * self)
{
	jive_buffer_resize(&self->contents, 0);
}

static void jive_section_fini(jive_section * self)
{
	jive_section_clear(self);
	jive_buffer_fini(&self->contents);
}

/* round up size of section to next multiple of 4096 (which is assumed
 to be the page size... */
static size_t jive_section_size_roundup(const jive_section * self)
{
	size_t size = self->contents.size;
	return (size + 4095) & ~4095;
}

void
jive_compilate_init(struct jive_compilate * self, struct jive_context * context)
{
	self->context = context;
	self->sections.first = self->sections.last = 0;
}

void
jive_compilate_fini(struct jive_compilate * self)
{
	jive_section * section, * saved_section;
	JIVE_LIST_ITERATE_SAFE(self->sections, section, saved_section, compilate_section_list) {
		jive_section_fini(section);
		jive_context_free(self->context, section);
	}
}

void
jive_compilate_clear(jive_compilate * self)
{
	jive_section * section;
	JIVE_LIST_ITERATE(self->sections, section, compilate_section_list) {
		jive_section_clear(section);
	}
}

jive_section *
jive_compilate_get_standard_section(jive_compilate * self,
	jive_stdsectionid sectionid)
{
	if ((int) sectionid <= 0)
		return NULL;
	
	jive_section * section;
	JIVE_LIST_ITERATE(self->sections, section, compilate_section_list) {
		if (section->id == sectionid)
			return section;
	}
	
	section = jive_context_malloc(self->context, sizeof(*section));
	jive_section_init(section, self->context, sectionid);
	JIVE_LIST_PUSH_BACK(self->sections, section, compilate_section_list);
	
	return section;
}

jive_buffer *
jive_compilate_get_buffer(struct jive_compilate * self, jive_stdsectionid id)
{
	jive_section * section = jive_compilate_get_standard_section(self, id);
	if (section)
		return &section->contents;
	else
		return 0;
}

void *
jive_compilate_map_to_memory(const jive_compilate * self)
{
	size_t size = 0;
	const jive_section * section;
	JIVE_LIST_ITERATE(self->sections, section, compilate_section_list) {
		size += jive_section_size_roundup(section);
	}
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
	
	if (ftruncate(fd, size) != 0) {
		close(fd);
		return 0;
	}
	
	void * writable = mmap(0, size, PROT_WRITE, MAP_SHARED, fd, 0);
	if (!writable) {
		close(fd);
		return 0;
	}
	
	size_t offset = 0;
	JIVE_LIST_ITERATE(self->sections, section, compilate_section_list) {
		memcpy(offset + (char *) writable,
		       section->contents.data, section->contents.size);
		offset += jive_section_size_roundup(section);
	}
	
	/* FIXME: generalize section loading a bit -- instead of going
	by id, should use something like section attributes */
	void * text_base = 0;
	void * data_base = 0;
	void * rodata_base = 0;
	void * bss_base = 0;
	offset = 0;
	JIVE_LIST_ITERATE(self->sections, section, compilate_section_list) {
		size_t size = jive_section_size_roundup(section);
		switch (section->id) {
			case jive_stdsectionid_code:
				text_base = mmap(0, size, PROT_READ|PROT_EXEC, MAP_SHARED,
						fd, offset);
				break;
			case jive_stdsectionid_data:
				/* FIXME: instead of remapping, could mprotect existing mapping */
				data_base = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED,
						fd, offset);
				break;
			case jive_stdsectionid_rodata:
				/* FIXME: instead of remapping, could mprotect existing mapping */
				rodata_base = mmap(0, size, PROT_READ, MAP_SHARED,
						fd, offset);
				break;
			case jive_stdsectionid_bss:
				/* FIXME: instead of remapping, could mprotect existing mapping */
				rodata_base = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED,
						fd, offset);
				break;
			default:
				break;
		}
		offset += jive_section_size_roundup(section);
	}
	
	/* FIXME: perform relocation here */
	
	/* FIXME: the following are basically inaccessible due to interface
	limitations so far */
	(void) data_base;
	(void) rodata_base;
	(void) bss_base;
	
	munmap(writable, size);
	close(fd);
	
	return text_base;;
}
