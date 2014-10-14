/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LOAD_H
#define JIVE_ARCH_LOAD_H

#include <memory>

#include <jive/util/ptr-collection.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/valuetype.h>

/* load normal form */

struct jive_load_node_normal_form;
struct jive_load_node_normal_form_class;

struct jive_load_node_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_load_node_normal_form * self, bool enable);
	jive::output * (*normalized_create)(const jive_load_node_normal_form * self,
		struct jive_region * region, const jive_node_attrs * attrs, jive::output * address,
		size_t nstates, jive::output * const states[]);
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

JIVE_EXPORTED_INLINE jive::output *
jive_load_node_normalized_create(const jive_load_node_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive::output * address,
	size_t nstates, jive::output * const states[])
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

class load_op final : public operation {
public:
	virtual
	~load_op() noexcept;

	template<typename Types>
	inline
	load_op(
		const jive::value::type & address_type,
		const Types & state_types,
		const jive::value::type & data_type)
		: address_type_(address_type.copy())
		, state_types_(detail::unique_ptr_vector_copy(state_types))
		, data_type_(data_type.copy())
	{
	}

	inline
	load_op(const load_op & other)
		: address_type_(other.address_type_->copy())
		, state_types_(detail::unique_ptr_vector_copy(other.state_types_))
		, data_type_(other.data_type_->copy())
	{
	}

	inline
	load_op(load_op && other) noexcept = default;

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

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline const jive::value::type &
	address_type() const noexcept { return *address_type_; }

	inline const std::vector<std::unique_ptr<jive::state::type>> &
	state_types() const noexcept { return state_types_; }

	inline const jive::value::type &
	data_type() const noexcept { return *data_type_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<jive::value::type> address_type_;
	std::vector<std::unique_ptr<jive::state::type>> state_types_;
	std::unique_ptr<jive::value::type> data_type_;
};

}

extern const jive_node_class JIVE_LOAD_NODE;

typedef jive::operation_node<jive::load_op> jive_load_node;

jive::output *
jive_load_by_address_create(jive::output * address,
	const jive::value::type * datatype,
	size_t nstates, jive::output * const states[]);

jive::output *
jive_load_by_bitstring_create(jive::output * address,
	size_t nbits, const jive::value::type * datatype,
	size_t nstates, jive::output * const states[]);

JIVE_EXPORTED_INLINE jive_load_node *
jive_load_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_LOAD_NODE))
		return (jive_load_node *) node;
	else
		return NULL;
}

#endif
