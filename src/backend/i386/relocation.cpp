/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/relocation.hpp>

bool
jive_i386_process_relocation(
	void * where, size_t max_size, jive_offset offset,
	jive_relocation_type type, jive_offset target, jive_offset value)
{
	target += value;
	/* FIXME: should honor endianness */
	switch (type.arch_code) {
		case 1: {/* JIVE_R_386_32 */
			if (max_size < 4)
				return false;
			uint32_t * loc = (uint32_t *) where;
			*loc = *loc + target;
			return true;
		}
		case 2: {/* JIVE_R_PC386_PC32 */
			if (max_size < 4)
				return false;
			uint32_t * loc = (uint32_t *) where;
			*loc = *loc + target - offset;
			return true;
		}
		case 20: {/* JIVE_R_386_16 */
			if (max_size < 2)
				return false;
			uint16_t * loc = (uint16_t *) where;
			*loc = *loc + target;
			return true;
		}
		case 21: {/* JIVE_R_386_PC16 */
			if (max_size < 2)
				return false;
			uint16_t * loc = (uint16_t *) where;
			*loc = *loc + target -offset;
			return true;
		}
		case 22: {/* JIVE_R_386_8 */
			if (max_size < 1)
				return false;
			uint8_t * loc = (uint8_t *) where;
			*loc = *loc + target;
			return true;
		}
		case 23: {/* JIVE_R_386_PC8 */
			if (max_size < 1)
				return false;
			uint8_t * loc = (uint8_t *) where;
			*loc = *loc + target - offset;
			return true;
		}
		default:
			return false;
	}
}
