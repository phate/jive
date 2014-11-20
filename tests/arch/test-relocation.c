/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/compilate.h>

static const jive_relocation_type ABS64 = {0};
static const jive_relocation_type REL64 = {1};

#include <stdio.h>

static bool 
process_relocation(
	void * where_, size_t max_size, jive_offset offset,
	jive_relocation_type type, jive_offset target, jive_offset value)
{
	int64_t * where = (int64_t *) where_;
	assert(max_size >= sizeof(int64_t));
	if (type.arch_code == 0) {
		*where = target;
		return true;
	} else if (type.arch_code == 1) {
		*where = target - offset;
		return true;
	} else {
		return false;
	}
}

static int test_main()
{
	jive_compilate compilate;
	jive_compilate_init(&compilate);
	
	jive_section * data = jive_compilate_get_standard_section(&compilate,
		jive_stdsectionid_data);
	jive_section * rodata = jive_compilate_get_standard_section(&compilate,
		jive_stdsectionid_rodata);
	
	int64_t value = 0;
	jive_section_put_reloc(data, &value, sizeof(value), ABS64,
		jive_symref_section(jive_stdsectionid_rodata),
		0);
	jive_section_put_reloc(rodata, &value, sizeof(value), REL64,
		jive_symref_section(jive_stdsectionid_data),
		0);
	
	jive_compilate_map * map = jive_compilate_load(&compilate,
		NULL,
		process_relocation);
	
	jive_compilate_fini(&compilate);
	
	uint64_t * data64 = (uint64_t *) map->sections[0].base;
	const uint64_t * rodata64 = (const uint64_t *) map->sections[1].base;
	
	assert(*data64 == (uintptr_t) rodata64);
	assert(*rodata64 == (uint64_t)((char *) data64 - (char *) rodata64));
	
	jive_compilate_map_unmap(map);
	
	jive_compilate_map_destroy(map);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-relocation", test_main);
