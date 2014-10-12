/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_PHI_NORMAL_FORM_H
#define JIVE_VSDG_PHI_NORMAL_FORM_H

#include <jive/vsdg/anchor-normal-form.h>

/* phi node normal form */

namespace jive {

class phi_normal_form final : public anchor_normal_form {
public:
	virtual
	~phi_normal_form() noexcept;

	phi_normal_form(
		const std::type_info & operator_class,
		const jive_node_class * node_class_old,
		jive::node_normal_form * parent,
		jive_graph * graph) noexcept;

	// FIXME: phi normal forms are totally broken.
};

}

#endif
