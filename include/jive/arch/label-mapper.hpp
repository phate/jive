/*
 * Copyright 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LABEL_MAPPER_HPP
#define JIVE_ARCH_LABEL_MAPPER_HPP

#include <jive/arch/linker-symbol.hpp>
#include <jive/rvsdg/label.hpp>

typedef struct jive_symbol_name_pair jive_symbol_name_pair;
struct jive_symbol_name_pair {
	const jive_linker_symbol * symbol;
	const char * name;
};

typedef struct jive_anon_label jive_anon_label;
struct jive_anon_label {
	const void * symbol;
	std::string name;
};


namespace jive {

class label_name_mapper {
public:
	virtual
	~label_name_mapper();

	virtual const char *
	map_named_symbol(const jive_linker_symbol * symbol) = 0;

	virtual const char *
	map_anon_symbol(const void * symbol) = 0;
};

class label_name_mapper_simple final : public label_name_mapper {
public:
	virtual
	~label_name_mapper_simple();

	label_name_mapper_simple(
		const jive_symbol_name_pair * pairs,
		size_t npairs)
	: label_name_mapper()
	, npairs_(npairs)
	, int_label_seqno_(0)
	, pairs_(pairs)
	{}

	virtual const char *
	map_named_symbol(const jive_linker_symbol * symbol) override;

	virtual const char *
	map_anon_symbol(const void * symbol) override;

private:
	size_t npairs_;
	size_t int_label_seqno_;
	const jive_symbol_name_pair * pairs_;
	std::unordered_map<const void*, struct jive_anon_label> anon_labels_;
};

}

#endif
