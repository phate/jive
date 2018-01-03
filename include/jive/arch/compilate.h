/*
 * Copyright 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_COMPILATE_H
#define JIVE_ARCH_COMPILATE_H

#include <stdint.h>

#include <jive/arch/linker-symbol.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/section.h>
#include <jive/util/buffer.h>

struct jive_linker_symbol_resolver;

typedef struct jive_compilate_map jive_compilate_map;
typedef struct jive_relocation_entry jive_relocation_entry;
typedef struct jive_relocation_type jive_relocation_type;
typedef struct jive_section jive_section;

/**
	\brief Architecture-specific representation of relocation type
*/
struct jive_relocation_type {
	uint32_t arch_code;
};

/**
	\brief Relocation table entry
*/
struct jive_relocation_entry {
	/** \brief Offset within section of relocation record */
	jive_offset offset;
	/** \brief Type of relocation to be applied */
	jive_relocation_type type;
	/** \brief Target address of relocation operation */
	jive_symref target;
	/** \brief Additional offset to be applied to symbol */
	jive_offset value;
};

/**
	\brief Apply relocation processing in a single location
	\param where Location of the data item
	\param max_size Maximum allowed size of data item
	\param offset Assumed offset of data item
	\param type Relocation type
	\param symoffset Offset of the symbol being referenced
	\param value Additional value from the relocation record
	\returns True if relocation could be applied
*/
typedef bool (*jive_process_relocation_function)(
	void * where, size_t max_size, jive_offset offset,
	jive_relocation_type type, jive_offset target, jive_offset value);

/**
	\brief Section of a compilate
*/
struct jive_section {
	jive_stdsectionid id;
	jive_buffer contents;

	std::unordered_set<jive_relocation_entry*> relocations;
};

static inline void
jive_section_put(jive_section * self, const void * data, size_t size)
{
	jive_buffer_put(&self->contents, data, size);
}

static inline void
jive_section_putbyte(jive_section * self, uint8_t byte)
{
	jive_buffer_putbyte(&self->contents, byte);
}

void
jive_section_put_reloc(jive_section * self, const void * data, size_t size,
	jive_relocation_type type, jive_symref target,
	jive_offset value);

namespace jive {

class compilate final {
public:
	~compilate();

	inline
	compilate()
	{}

	compilate(const compilate &) = delete;

	compilate(compilate &&) = delete;

	compilate &
	operator=(const compilate &) = delete;

	compilate &
	operator=(compilate &&) = delete;

	/**
		\brief Clear compilation object

		Clears the contents of the given compilation object, i.e. subsequently
		it behaves as if it were newly allocated (actual buffers allocated
		might be reused as an optimization, though).
	*/
	void
	clear();

	jive_section *
	section(jive_stdsectionid sectionid);

	inline jive_buffer *
	buffer(jive_stdsectionid id)
	{
		auto s = section(id);
		return s ? &s->contents : nullptr;
	}

	/**
		\brief Load a compilate into process' address space

		\returns A description of where sections have been mapped

		Maps all of the sections contained in the compilate into the process'
		address space. Returns a structure describing the mapping of the
		sections to the address space.
	*/
	jive_compilate_map *
	load(
		const struct jive_linker_symbol_resolver * sym_resolver,
		jive_process_relocation_function relocate);

private:
	std::vector<std::unique_ptr<jive_section>> sections_;
};

}

class jive_compilate_section {
public:
	inline
	jive_compilate_section(const jive_section * _section, void * _base, size_t _size)
		: section(_section)
		, base(_base)
		, size(_size)
	{}

	/** \brief Section descriptor from compilate */
	const jive_section * section;
	/** \brief Base address in process' address space */
	void * base;
	/** \brief Size of loaded section */
	size_t size;
};

/** \brief Represent a compilate loaded into process' address space */
struct jive_compilate_map {
	std::vector<jive_compilate_section> sections;
};


/**
	\brief Destroy a compilate map
	\param self The compilate map to be destroyed
	
	Destroys a compilate map, freeing all allocated memory used to
	represent the map itself. The memory regions described by this
	map are *not* affected, see \ref jive_compilate_map_unmap
*/
void
jive_compilate_map_destroy(jive_compilate_map * self);

/**
	\brief Lookup a section base in a mapped compilate
	\param self The compilate map describing the loaded compilate
	\param id The standard section id of the requested section
	
	Lookup the address where the start of a specific section has been
	mapped into the process' address space. Returns NULL if no such
	section has been mapped.
*/
static inline void *
jive_compilate_map_get_stdsection(const jive_compilate_map * self, jive_stdsectionid id)
{
	size_t n;
	for (n = 0; n < self->sections.size(); ++n) {
		if (self->sections[n].section->id == id)
			return self->sections[n].base;
	}
	return NULL;
}

/**
	\brief Unmap a mapping of a compilate
	\param self Mapping to be removed
	
	Unmaps the mapping described by a compilate map. The data structure
	representing the mapping itself will not be freed, see
	\ref jive_compilate_map_destroy
*/
void
jive_compilate_map_unmap(const jive_compilate_map * self);

#endif
