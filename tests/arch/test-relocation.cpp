/*
 * Copyright 2012 2013 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/compilate.hpp>

static const jive_relocation_type ABS64 = {0};
static const jive_relocation_type REL64 = {1};

#include <stdio.h>

static bool
process_relocation(
	void * where_, size_t max_size, jive_offset offset,
	jive_relocation_type type, jive_offset target, jive_offset value)
{
	uint64_t * where = (uint64_t *) where_;
	assert(max_size >= sizeof(uint64_t));
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
	jive::compilate compilate;

	auto data = compilate.section(jive_stdsectionid_data);
	auto rodata = compilate.section(jive_stdsectionid_rodata);
	
	int64_t value = 0;
	data->add_relocation(&value, sizeof(value), ABS64,
		jive_symref_section(jive_stdsectionid_rodata), 0);
	rodata->add_relocation(&value, sizeof(value), REL64,
		jive_symref_section(jive_stdsectionid_data), 0);
	
	auto map = compilate.load(nullptr, process_relocation);
	
	uint64_t * data64 = (uint64_t *) map->section(jive_stdsectionid_data);
	const uint64_t * rodata64 = (const uint64_t *) map->section(jive_stdsectionid_rodata);
	
	assert(*data64 == (uintptr_t) rodata64);
	assert(*rodata64 == (uint64_t)((char *) data64 - (char *) rodata64));
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-relocation", test_main)
