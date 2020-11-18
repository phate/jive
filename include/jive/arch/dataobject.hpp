/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_DATAOBJECT_HPP
#define JIVE_ARCH_DATAOBJECT_HPP

#include <jive/common.hpp>

#include <jive/rvsdg/region.hpp>
#include <jive/rvsdg/structural-node.hpp>
#include <jive/util/ptr-collection.hpp>

namespace jive {

class dataobj_op final : public structural_op {
public:
	virtual
	~dataobj_op() noexcept;

	inline
	dataobj_op(const std::vector<std::unique_ptr<const jive::type>> & types)
	: types_(detail::unique_ptr_vector_copy(types))
	{}

	inline
	dataobj_op(std::vector<std::unique_ptr<const jive::type>> && types)
	: types_(std::move(types))
	{}

	inline
	dataobj_op(const dataobj_op & other)
	: types_(detail::unique_ptr_vector_copy(other.types_))
	{}

	inline
	dataobj_op(dataobj_op && other) noexcept = default;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<std::unique_ptr<const jive::type>> types_;
};

class dataobj_node final : public structural_node {
public:
	~dataobj_node() override;

private:
	dataobj_node(
		jive::region * parent,
		const std::vector<std::unique_ptr<const jive::type>> & types)
	: structural_node(dataobj_op(types), parent, 1)
	{}

public:
	static dataobj_node *
	create(
		jive::region * parent,
		const std::vector<std::unique_ptr<const jive::type>> & types)
	{
		return new dataobj_node(parent, types);
	}
};

class memlayout_mapper;

}

jive::output *
jive_dataobj(jive::output * data, jive::memlayout_mapper * mapper);

jive::output *
jive_rodataobj(jive::output * data, jive::memlayout_mapper * mapper);

jive::output *
jive_bssobj(jive::output * data, jive::memlayout_mapper * mapper);

#endif
