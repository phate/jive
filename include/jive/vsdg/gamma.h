#ifndef JIVE_VSDG_GAMMA_H
#define JIVE_VSDG_GAMMA_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node.h>

struct jive_graph;
struct jive_output;
struct jive_type;
struct jive_region;

extern const jive_node_class JIVE_GAMMA_TAIL_NODE;
extern const jive_node_class JIVE_GAMMA_NODE;

struct jive_output * const *
jive_choose(struct jive_output * predicate,
	size_t nvalues, const struct jive_type * types[const],
	struct jive_output * const true_values[],
	struct jive_output * const false_values[const]);

struct jive_node *
jive_gamma_create(
	struct jive_region * region,
	struct jive_output * predicate,
	size_t nvalues, const struct jive_type * types[const],
	struct jive_output * const true_values[],
	struct jive_output * const false_values[]);

/* normal form */

typedef struct jive_gamma_normal_form_class jive_gamma_normal_form_class;
typedef struct jive_gamma_normal_form jive_gamma_normal_form;

struct jive_gamma_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_predicate_reduction)(jive_gamma_normal_form * self, bool enable);
	void (*set_invariant_reduction)(jive_gamma_normal_form * self, bool enable);
};

struct jive_gamma_normal_form {
	jive_node_normal_form base;
	bool enable_predicate_reduction;
	bool enable_invariant_reduction;
};

extern const jive_gamma_normal_form_class JIVE_GAMMA_NORMAL_FORM_;
#define JIVE_GAMMA_NORMAL_FORM (JIVE_GAMMA_NORMAL_FORM_.base)

JIVE_EXPORTED_INLINE void
jive_gamma_normal_form_set_predicate_reduction(jive_gamma_normal_form * self, bool enable)
{
	const jive_gamma_normal_form_class * cls;
	cls = (const jive_gamma_normal_form_class *) self->base.class_;
	cls->set_predicate_reduction(self, enable);
}

JIVE_EXPORTED_INLINE void
jive_gamma_normal_form_set_invariant_reduction(jive_gamma_normal_form * self, bool enable)
{
	const jive_gamma_normal_form_class * cls;
	cls = (const jive_gamma_normal_form_class *) self->base.class_;
	cls->set_invariant_reduction(self, enable);
}

JIVE_EXPORTED_INLINE jive_gamma_normal_form *
jive_gamma_normal_form_cast(jive_node_normal_form * self)
{
	const jive_node_normal_form_class * cls = self->class_;
	while (cls) {
		if (cls == &JIVE_GAMMA_NORMAL_FORM)
			return (jive_gamma_normal_form *) self;
		cls = cls->parent;
	}
	return 0;
}

/* protected virtual methods */

bool
jive_gamma_normal_form_normalize_node_(const jive_node_normal_form * self, jive_node * node);

bool
jive_gamma_normal_form_operands_are_normalized_(const jive_node_normal_form * self,
	size_t noperands, jive_output * const operands[],
	const jive_node_attrs * attrs);

void
jive_gamma_normal_form_init_(jive_gamma_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_,
	jive_graph * graph);
	
void
jive_gamma_normal_form_class_set_predicate_reduction_(jive_gamma_normal_form * self, bool enable);

void
jive_gamma_normal_form_class_set_invariant_reduction_(jive_gamma_normal_form * self, bool enable);

#endif
