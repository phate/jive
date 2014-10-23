/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/value-representation.h>

#include <stdexcept>

namespace jive {
namespace bits {

uint64_t
value_repr_to_uint(const value_repr & value)
{
	size_t limit = std::min(value.size(), size_t(64));
	/* bits beyond 64 must be zero, else value is not representable as uint64_t */
	for (size_t n = limit; n < value.size(); ++n) {
		if (value[n] != '0') {
			throw std::range_error("Bit constant value exceeds uint64 range");
		}
	}

	uint64_t result = 0;
	uint64_t pos_value = 1;
	for (size_t n = 0; n < limit; ++n) {
		switch (value[n]) {
			case '0': {
				break;
			}
			case '1': {
				result |= pos_value;
				break;
			}
			default: {
				throw std::range_error("Undetermined bit constant");
			}
		}
		pos_value = pos_value << 1;
	}
	return result;
}

int64_t
value_repr_to_int(const value_repr & value)
{
	if (value.empty()) {
		return 0;
	}

	/* all bits from 63 on must be identical, else value is not representable as int64_t */
	char sign_bit = *value.rbegin();
	size_t limit = std::min(value.size(), size_t(63));
	for (size_t n = limit; n < value.size(); ++n) {
		if (value[n] != sign_bit) {
			throw std::range_error("Bit constant value exceeds int64 range");
		}
	}

	int64_t result = 0;
	uint64_t pos_value = 1;
	for (size_t n = 0; n < 64; ++n) {
		switch (n < value.size() ? value[n] : sign_bit) {
			case '0': {
				break;
			}
			case '1': {
				result |= pos_value;
				break;
			}
			default: {
				throw std::range_error("Undetermined bit constant");
			}
		}
		pos_value = pos_value << 1;
	}
	return result;
}

}
}
