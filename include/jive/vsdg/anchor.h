/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHOR_H
#define JIVE_VSDG_ANCHOR_H

#include <jive/vsdg/node.h>

namespace jive {

class region_head_op : public operation {
public:
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;
};

class region_tail_op : public operation {
public:
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;
};

class region_anchor_op : public operation {
public:
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;
};

}

/* anchor node */

typedef struct jive_node_class jive_anchor_node_class;

extern const jive_anchor_node_class JIVE_ANCHOR_NODE;

/* anchor node normal form */

typedef struct jive_anchor_node_normal_form jive_anchor_node_normal_form;
typedef struct jive_anchor_node_normal_form_class jive_anchor_node_normal_form_class;

struct jive_anchor_node_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_anchor_node_normal_form * self, bool enable);
};

extern const jive_anchor_node_normal_form_class JIVE_ANCHOR_NODE_NORMAL_FORM_;
#define JIVE_ANCHOR_NODE_NORMAL_FORM (JIVE_ANCHOR_NODE_NORMAL_FORM_.base)

struct jive_anchor_node_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
};

JIVE_EXPORTED_INLINE jive_anchor_node_normal_form *
jive_anchor_node_normal_form_cast(jive_node_normal_form * self)
{
	if (jive_node_normal_form_isinstance(self, &JIVE_ANCHOR_NODE_NORMAL_FORM))
		return (jive_anchor_node_normal_form *) self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE void
jive_anchor_node_normal_form_set_reducible(jive_anchor_node_normal_form * self, bool reducible)
{
	const jive_anchor_node_normal_form_class * cls;
	cls = (const jive_anchor_node_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);
}

#endif
