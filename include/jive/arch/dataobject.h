/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_DATAOBJECT_H
#define JIVE_ARCH_DATAOBJECT_H

#include <jive/common.h>

#include <jive/util/ptr-collection.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {

class dataobj_head_op final : public region_head_op {
public:
	virtual
	~dataobj_head_op() noexcept;

	template<typename Container>
	dataobj_head_op(const Container & container)
		: types_(detail::unique_ptr_vector_copy(container))
	{
	}

	inline
	dataobj_head_op(std::vector<std::unique_ptr<const base::type>> && types)
		: types_(std::move(types))
	{
	}

	inline
	dataobj_head_op(const dataobj_head_op & other)
		: types_(detail::unique_ptr_vector_copy(other.types_))
	{
	}

	inline
	dataobj_head_op(dataobj_head_op && other) noexcept = default;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<std::unique_ptr<const base::type>> types_;
};

class dataobj_tail_op final : public region_tail_op {
public:
	virtual
	~dataobj_tail_op() noexcept;

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

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class dataobj_op final : public region_anchor_op {
public:
	virtual
	~dataobj_op() noexcept;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}

struct jive_memlayout_mapper;

typedef jive::operation_node<jive::dataobj_head_op> jive_dataitems_node;
typedef jive::operation_node<jive::dataobj_tail_op> jive_datadef_node;
typedef jive::operation_node<jive::dataobj_op> jive_dataobj_node;

jive::output *
jive_dataobj(jive::output * data, jive_memlayout_mapper * mapper);

jive::output *
jive_rodataobj(jive::output * data, jive_memlayout_mapper * mapper);

jive::output *
jive_bssobj(jive::output * data, jive_memlayout_mapper * mapper);

#endif
