/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHOR_NORMAL_FORM_H
#define JIVE_VSDG_ANCHOR_NORMAL_FORM_H

#include <jive/vsdg/node-normal-form.h>

namespace jive {

class anchor_normal_form : public node_normal_form {
public:
	virtual
	~anchor_normal_form() noexcept;

	anchor_normal_form(
		const jive_node_class * node_class,
		jive::node_normal_form * parent,
		jive_graph * graph) noexcept;

	virtual void
	set_reducible(bool enable);
	inline bool
	get_reducible() const noexcept { return enable_reducible_; }

private:
	bool enable_reducible_;
};

}

/* anchor node normal form */

extern const jive_node_normal_form_class JIVE_ANCHOR_NODE_NORMAL_FORM;

#endif
