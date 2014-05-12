/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OBJDEF_H
#define JIVE_VSDG_OBJDEF_H

#include <string>

#include <jive/vsdg/node.h>

struct jive_label;
struct jive_linker_symbol;
struct jive_output;

namespace jive {

class objdef_operation final : public operation {
public:
	inline objdef_operation(
		const std::string & name,
		const jive_linker_symbol * symbol)
		: name_(name)
		, symbol_(symbol)
	{
	}

	const std::string & name() const noexcept { return name_; }
	const jive_linker_symbol * symbol() const noexcept { return symbol_; }
private:
	std::string name_;
	const jive_linker_symbol * symbol_;
};

}

extern const struct jive_node_class JIVE_OBJDEF_NODE;

typedef jive::operation_node<jive::objdef_operation> jive_objdef_node;

struct jive_node *
jive_objdef_node_create(
	struct jive_output * output,
	const char * name,
	const struct jive_linker_symbol * symbol);

JIVE_EXPORTED_INLINE jive_objdef_node *
jive_objdef_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_OBJDEF_NODE)
		return (jive_objdef_node *) node;
	else
		return NULL;
}

#endif
