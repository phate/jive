/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTSYMBOLIC_H
#define JIVE_TYPES_FUNCTION_FCTSYMBOLIC_H

#include <string>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace fct {

class symfunction_operation final : public jive::operation {
public:
	virtual ~symfunction_operation() noexcept;

	symfunction_operation(
		const std::string & name,
		const jive::fct::type & type);

	symfunction_operation(
		const std::string && name,
		const jive::fct::type && type) noexcept;

	inline const std::string& name() const noexcept { return name_; }
	inline const jive::fct::type& type() const noexcept { return type_; }
private:
	std::string name_;
	jive::fct::type type_;
};

}
}

typedef jive::operation_node<jive::fct::symfunction_operation> jive_symbolicfunction_node;

extern const jive_node_class JIVE_SYMBOLICFUNCTION_NODE;

jive_node *
jive_symbolicfunction_node_create(
	struct jive_graph * graph, const char * name, const jive::fct::type * type);

jive::output *
jive_symbolicfunction_create(
	struct jive_graph * graph, const char * name, const jive::fct::type * type);

JIVE_EXPORTED_INLINE jive_symbolicfunction_node *
jive_symbolicfunction_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_SYMBOLICFUNCTION_NODE) return (jive_symbolicfunction_node *) node;
	else return 0;
}

#endif
