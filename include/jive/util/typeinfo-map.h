/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */
#ifndef JIVE_UTIL_TYPEINFO_MAP_H
#define JIVE_UTIL_TYPEINFO_MAP_H

/*
 * Auxiliary internal class template typeinfo_map<T>, to provide a mapping
 * from type_info to other objects.
 */

#include <unordered_map>
#include <typeinfo>

namespace jive {
namespace detail {

class type_info_ptr_hash {
public:
	inline size_t operator()(
		const std::type_info * obj) const noexcept
	{
		return obj->hash_code();
	}
};

class type_info_ptr_eq {
public:
	inline bool operator()(
		const std::type_info * obj1,
		const std::type_info * obj2) const noexcept
	{
		return *obj1 == *obj2;
	}
};

template<typename T>
using typeinfo_map = std::unordered_map<const std::type_info *, T, type_info_ptr_hash, type_info_ptr_eq>;

}
}

#endif
