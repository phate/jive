/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GAMMA_H
#define JIVE_VSDG_GAMMA_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>

struct jive_graph;
struct jive_region;

namespace jive {
	class output;
	class type;
}

extern const jive_node_class JIVE_GAMMA_TAIL_NODE;
extern const jive_node_class JIVE_GAMMA_NODE;

namespace jive {

class gamma_tail_op final : public region_tail_op {
public:
	virtual
	~gamma_tail_op() noexcept;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;
};

class gamma_op final : public region_anchor_op {
public:
	virtual
	~gamma_op() noexcept;

	inline constexpr
	gamma_op(size_t nalternatives = 2) noexcept
		: nalternatives_(nalternatives)
	{
	}

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

private:
	size_t nalternatives_;
};

}

void
jive_gamma(jive::output * predicate,
	size_t nvalues, const struct jive::base::type * const types[],
	jive::output * const true_values[],
	jive::output * const false_values[],
	jive::output * results[]);

/* normal form */

typedef struct jive_gamma_normal_form jive_gamma_normal_form;
typedef struct jive_gamma_normal_form_class jive_gamma_normal_form_class;

struct jive_gamma_normal_form_class {
	jive_anchor_node_normal_form_class base;
	void (*set_predicate_reduction)(jive_gamma_normal_form * self, bool enable);
	void (*set_invariant_reduction)(jive_gamma_normal_form * self, bool enable);
};

struct jive_gamma_normal_form {
	jive_anchor_node_normal_form base;
	bool enable_predicate_reduction;
	bool enable_invariant_reduction;
};

extern const jive_gamma_normal_form_class JIVE_GAMMA_NORMAL_FORM_;
#define JIVE_GAMMA_NORMAL_FORM (JIVE_GAMMA_NORMAL_FORM_.base.base)

JIVE_EXPORTED_INLINE void
jive_gamma_normal_form_set_predicate_reduction(jive_gamma_normal_form * self, bool enable)
{
	const jive_gamma_normal_form_class * cls;
	cls = (const jive_gamma_normal_form_class *) self->base.base.class_;
	cls->set_predicate_reduction(self, enable);
}

JIVE_EXPORTED_INLINE void
jive_gamma_normal_form_set_invariant_reduction(jive_gamma_normal_form * self, bool enable)
{
	const jive_gamma_normal_form_class * cls;
	cls = (const jive_gamma_normal_form_class *) self->base.base.class_;
	cls->set_invariant_reduction(self, enable);
}

JIVE_EXPORTED_INLINE jive_gamma_normal_form *
jive_gamma_normal_form_cast(jive_node_normal_form * self)
{
	if (jive_node_normal_form_isinstance(self, &JIVE_GAMMA_NORMAL_FORM))
		return (jive_gamma_normal_form *) self;
	else
		return NULL;
}

/* protected virtual methods */

bool
jive_gamma_normal_form_normalize_node_(const jive_node_normal_form * self, jive_node * node);

bool
jive_gamma_normal_form_operands_are_normalized_(const jive_node_normal_form * self,
	size_t noperands, jive::output * const operands[],
	const jive_node_attrs * attrs);

void
jive_gamma_normal_form_init_(jive_gamma_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_,
	jive_graph * graph);

void
jive_gamma_normal_form_class_set_reducible_(jive_anchor_node_normal_form * self, bool reducible);
	
void
jive_gamma_normal_form_class_set_predicate_reduction_(jive_gamma_normal_form * self, bool enable);

void
jive_gamma_normal_form_class_set_invariant_reduction_(jive_gamma_normal_form * self, bool enable);

#endif
