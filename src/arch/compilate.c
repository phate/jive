/*
 * Copyright 2012 2013 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int
get_tmpfd(size_t size)
{
	char filename_template[] = "/tmp/jive-exec-buffer-XXXXXX";
#if defined(_GNU_SOURCE) && defined(O_CLOEXEC)
	int fd = mkostemp(filename_template, O_CLOEXEC);
#else
	int fd = mkstemp(filename_template);
	if (fd >= 0) fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
	if (fd < 0)
		return -1;
	unlink(filename_template);
	
	if (ftruncate(fd, size) != 0) {
		close(fd);
		return fd = -1;
	}
	return fd;
}

static bool
resolve_relocation_target(
	jive_symref target,
	const jive_compilate_map * map,
	const jive_linker_symbol_resolver * sym_resolver,
	const void ** resolved)
{
	switch (target.type) {
		case jive_symref_type_section: {
			size_t n;
			for (n = 0; n < map->sections.size(); ++n) {
				if (map->sections[n].section->id() == target.ref.section) {
					*resolved = map->sections[n].base;
					return true;
				}
			}
			return false;
		}
		case jive_symref_type_linker_symbol: {
			return jive_linker_symbol_resolver_resolve(sym_resolver, target.ref.linker_symbol, resolved);
		}
		case jive_symref_type_none: {
			/* This case can occur when an absolute value (i.e.:
			zero-based offset) is to be put into an instruction that
			requires pc-relative encoding for that operand; just
			returning zero may seem odd, but is correct. */
			*resolved = 0;
			return true;
		}
	}
	
	return false;
}

static bool
section_process_relocations(
	void * base_writable,
	jive_offset base,
	const jive_compilate_map * map,
	const jive::section * section,
	const jive_linker_symbol_resolver * sym_resolver,
	jive_process_relocation_function relocate)
{
	for (auto & entry : section->relocations) {
		void * where = entry->offset() + (char *) base_writable;
		jive_offset offset = entry->offset() + base;
		const void * target;
		if (!resolve_relocation_target(entry->target(), map, sym_resolver, &target))
			return false;
		if (!relocate(where, section->size() - entry->offset(),
			offset, entry->type(), (uintptr_t) target, entry->value())) {
			return false;
		}
	}
	
	return true;
}

/* round up size of section to next multiple of 4096 (which is assumed
 to be the page size... */
static size_t
jive_section_size_roundup(const jive::section * self)
{
	return (self->size() + 4095) & ~4095;
}

namespace jive {

/* section */

void
section::add_relocation(
	const void * data,
	size_t size,
	jive_relocation_type type,
	jive_symref target,
	jive_offset value)
{
	relocations.insert(new relocation_entry(this->size(), type, target, value));
	put(data, size);
}

/* compilate */

compilate::~compilate()
{
	sections_.clear();
}

void
compilate::clear()
{
	for (auto & section : sections_)
		section->clear();
}

jive::section *
compilate::section(jive_stdsectionid sectionid)
{
	if ((int) sectionid <= 0)
		return nullptr;

	for (const auto & section : sections_) {
		if (section->id() == sectionid)
			return section.get();
	}

	sections_.push_back(std::make_unique<jive::section>(sectionid));

	return sections_.back().get();
}

jive_compilate_map *
compilate::load(
	const jive_linker_symbol_resolver * sym_resolver,
	jive_process_relocation_function relocate)
{
	size_t total_size = 0;
	for (const auto & section : sections_)
		total_size += jive_section_size_roundup(section.get());

	int fd = get_tmpfd(total_size);
	if (fd == -1)
		return NULL;
	
	jive_compilate_map * map = new jive_compilate_map;
	if (!map) {
		close(fd);
		return NULL;
	}
	
	void * writable = mmap(0, total_size, PROT_WRITE, MAP_SHARED, fd, 0);
	if (!writable) {
		close(fd);
		jive_compilate_map_destroy(map);
		return 0;
	}
	
	/* populate all sections with data, but keep them writable independent
	of their designation, so relocation processing can modify their
	contents */
	size_t offset = 0;
	for (const auto & section : sections_) {
		void * addr = offset + (char *) writable;
		map->sections.push_back(jive_compilate_section(section.get(), addr,
			jive_section_size_roundup(section.get())));
		size_t n = map->sections.size()-1;
		
		memcpy(addr, &section->buffer()->data[0], section->size());
		
		/* If this is a code section, create another mapping, this time
		executable. We cannot generally assume that we can later change
		an existing mapping to executable (hello PaX, hello SELinux),
		or that the same address range through which code was written
		is also suitable for execution (hello PowerPC). Creating a
		separate mapping allows the kernel to set things up properly. */
		/* FIXME: use section attributes instead of id to decide
		whether section should be executable. */
		if (section->id() == jive_stdsectionid_code) {
			void * exec_addr = mmap(0, map->sections[n].size,
				PROT_READ|PROT_EXEC, MAP_SHARED, fd, offset);
			map->sections[n].base = exec_addr;
		}
		
		offset += map->sections[n].size;
	}
	
	/* finalize all sections and switch them over to their correct
	permissions */
	offset = 0;
	size_t n = 0;
	bool success = true;
	for (const auto & section : sections_) {
		void * base = offset + (char *) writable;
		
		success = success && section_process_relocations(base,
			(jive_offset) (uintptr_t) map->sections[n].base,
			map, section.get(), sym_resolver, relocate);
		
		switch (section->id()) {
			case jive_stdsectionid_code: {
				munmap(base, map->sections[n].size);
				/* The contents of the memory region might
				have been changed, the following should force
				synchronization of the icache. */
				void * exec_addr = (void *) (intptr_t)
					map->sections[n].base;
				mprotect(exec_addr, map->sections[n].size,
					PROT_NONE);
				mprotect(exec_addr, map->sections[n].size,
					PROT_READ|PROT_EXEC);
				break;
			}
			case jive_stdsectionid_rodata: {
				mprotect(base, map->sections[n].size,
					PROT_READ);
				break;
			}
			default: {
				/* empty */
			}
		}
		
		offset += map->sections[n].size;
		++n;
	}
	
	if (!success) {
		jive_compilate_map_unmap(map);
		jive_compilate_map_destroy(map);
		return NULL;
	}
	
	return map;
}

}	//jive namespace

void
jive_compilate_map_destroy(jive_compilate_map * self)
{
	delete self;
}

void
jive_compilate_map_unmap(const jive_compilate_map * self)
{
	size_t n;
	for (n = 0; n < self->sections.size(); ++n) {
		void * ptr = (void *) (intptr_t) self->sections[n].base;
		munmap(ptr, self->sections[n].size);
	}
}
