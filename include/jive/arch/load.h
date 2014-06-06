/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LOAD_H
#define JIVE_ARCH_LOAD_H

#include <memory>

#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype.h>

/* load normal form */

struct jive_load_node_normal_form;
struct jive_load_node_normal_form_class;

struct jive_load_node_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_load_node_normal_form * self, bool enable);
	jive_output * (*normalized_create)(const jive_load_node_normal_form * self,
		struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
		size_t nstates, jive_output * const states[]);
};

extern const jive_load_node_normal_form_class JIVE_LOAD_NODE_NORMAL_FORM_;
#define JIVE_LOAD_NODE_NORMAL_FORM (JIVE_LOAD_NODE_NORMAL_FORM_.base)

struct jive_load_node_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
};

JIVE_EXPORTED_INLINE jive_load_node_normal_form *
jive_load_node_normal_form_cast(jive_node_normal_form * self)
{
	if (jive_node_normal_form_isinstance(self, &JIVE_LOAD_NODE_NORMAL_FORM))
		return (jive_load_node_normal_form *) self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_output *
jive_load_node_normalized_create(const jive_load_node_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
	size_t nstates, jive_output * const states[])
{
	const jive_load_node_normal_form_class * cls;
	cls = (const jive_load_node_normal_form_class *) self->base.class_;

	return cls->normalized_create(self, region, attrs, address, nstates, states);
}

JIVE_EXPORTED_INLINE void
jive_load_node_set_reducible(jive_load_node_normal_form * self, bool reducible)
{
	const jive_load_node_normal_form_class * cls;
	cls = (const jive_load_node_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);
}

/* load node */

namespace jive {

class load_operation final : public operation {
public:
	inline
	load_operation(size_t nbits, const jive::value::type * datatype)
		: nbits_(nbits)
		, datatype_(datatype->copy())
	{
	}

	inline
	load_operation(const load_operation & other)
		: nbits_(other.nbits())
		, datatype_(other.datatype().copy())
	{
	}

	inline
	load_operation(load_operation && other) noexcept = default;

	inline size_t nbits() const noexcept { return nbits_; }
	inline const jive::value::type & datatype() const noexcept { return *datatype_; }
private:
	size_t nbits_;
	std::unique_ptr<jive::value::type> datatype_;
};

}

extern const jive_node_class JIVE_LOAD_NODE;

typedef jive::operation_node<jive::load_operation> jive_load_node;

struct jive_node *
jive_load_by_address_node_create(struct jive_region * region,
	struct jive_output * address,
	const jive::value::type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_by_address_create(struct jive_output * address,
	const jive::value::type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_node *
jive_load_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * address, size_t nbits,
	const jive::value::type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_by_bitstring_create(struct jive_output * address,
	size_t nbits, const jive::value::type * datatype,
	size_t nstates, struct jive_output * const states[]);

JIVE_EXPORTED_INLINE jive_load_node *
jive_load_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_LOAD_NODE))
		return (jive_load_node *) node;
	else
		return NULL;
}

#endif
