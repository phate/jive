/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SECTION_H
#define JIVE_VSDG_SECTION_H

typedef enum jive_stdsectionid {
	jive_stdsectionid_invalid = 0,
	jive_stdsectionid_external = 1,
	jive_stdsectionid_code = 2,
	jive_stdsectionid_data = 3,
	jive_stdsectionid_rodata = 4,
	jive_stdsectionid_bss = 5
} jive_stdsectionid;

#endif
