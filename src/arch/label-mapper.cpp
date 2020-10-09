/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/label-mapper.hpp>

namespace jive {

/* label name mapper */

label_name_mapper::~label_name_mapper()
{}

/* label name mapper simple */

label_name_mapper_simple::~label_name_mapper_simple()
{}

const char *
label_name_mapper_simple::map_named_symbol(const jive_linker_symbol * symbol)
{
	for (size_t n = 0; n < npairs_; n++) {
		if (pairs_[n].symbol == symbol)
			return pairs_[n].name;
	}

	return nullptr;
}

const char *
label_name_mapper_simple::map_anon_symbol(const void * symbol)
{
	jive_anon_label entry;
	auto i = anon_labels_.find(symbol);
	if (i != anon_labels_.end())
		entry = i->second;
	else {
		entry.symbol = symbol;
		entry.name = jive::detail::strfmt(".L", ++int_label_seqno_);
		anon_labels_[symbol] = entry;
	}
	
	return entry.name.c_str();
}

}
