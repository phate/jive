/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_SIMPLE_NORMAL_FORM_H
#define JIVE_VSDG_OPERATORS_SIMPLE_NORMAL_FORM_H

#include <jive/vsdg/node-normal-form.h>

namespace jive {

class simple_normal_form : public node_normal_form {
public:
	virtual
	~simple_normal_form() noexcept;

	simple_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph) noexcept;
};

}

#endif
