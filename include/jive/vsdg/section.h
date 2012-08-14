#ifndef JIVE_VSDG_SECTION_H
#define JIVE_VSDG_SECTION_H

typedef enum jive_section {
	jive_section_invalid = 0,
	jive_section_external = 1,
	jive_section_code = 2,
	jive_section_data = 3,
	jive_section_rodata = 4,
	jive_section_bss = 5
} jive_section;

#endif
