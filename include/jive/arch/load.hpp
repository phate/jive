/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LOAD_HPP
#define JIVE_ARCH_LOAD_HPP

#include <jive/arch/addresstype.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/node-normal-form.hpp>
#include <jive/rvsdg/node.hpp>
#include <jive/rvsdg/simple-node.hpp>
#include <jive/rvsdg/simple-normal-form.hpp>
#include <jive/types/bitstring/type.hpp>

namespace jive {

/* load normal form */

class load_normal_form : public simple_normal_form {
public:
	virtual
	~load_normal_form() noexcept;

	load_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph) noexcept;

	virtual bool
	normalize_node(jive::node * node) const override;

	virtual std::vector<jive::output*>
	normalized_create(
		jive::region * region,
		const jive::simple_op & op,
		const std::vector<jive::output*> & arguments) const override;

	virtual void
	set_reducible(bool enable);
	inline bool
	get_reducible() const noexcept { return enable_reducible_; }

private:
	bool enable_reducible_;
};

/* load operator */

class load_op : public simple_op {
public:
	virtual
	~load_op() noexcept;

protected:
	inline
	load_op(
		const jive::valuetype & address,
		const jive::valuetype & value,
		size_t nstates)
	: simple_op(create_operands(address, nstates), {value})
	{}

public:
	inline const jive::valuetype &
	addresstype() const noexcept
	{
		return *static_cast<const jive::valuetype*>(&argument(0).type());
	}

	inline const jive::valuetype &
	valuetype() const noexcept
	{
		return *static_cast<const jive::valuetype*>(&result(0).type());
	}

	virtual bool
	operator==(const operation & other) const noexcept override final;

	static inline jive::load_normal_form *
	normal_form(jive::graph * graph) noexcept
	{
		return static_cast<load_normal_form*>(graph->node_normal_form(typeid(load_op)));
	}

private:
	static std::vector<jive::port>
	create_operands(const jive::valuetype & address, size_t nstates);
};

/* address load operator */

class addrload_op final : public load_op {
public:
	virtual
	~addrload_op();

	inline
	addrload_op(
		const jive::addrtype & address,
		size_t nstates)
	: load_op(address, address.type(), nstates)
	{}

	inline const jive::addrtype &
	addresstype() const noexcept
	{
		return *static_cast<const addrtype*>(&load_op::addresstype());
	}

	virtual std::string
	debug_string() const override;

	std::unique_ptr<operation>
	copy() const override;

	static inline jive::output *
	create(
		jive::output * address,
		const std::vector<jive::output*> & states)
	{
		auto at = dynamic_cast<const addrtype*>(&address->type());
		if (!at) throw type_error("address", address->type().debug_string());

		std::vector<jive::output*> operands(1, address);
		operands.insert(operands.end(), states.begin(), states.end());

		addrload_op op(*at, states.size());
		return simple_node::create_normalized(address->region(), op, operands)[0];
	}
};

/* bitstring load operator */

class bitload_op final : public load_op {
public:
	virtual
	~bitload_op();

	inline
	bitload_op(
		const bittype & address,
		const jive::valuetype & value,
		size_t nstates)
	: load_op(address, value, nstates)
	{}

	inline const bittype &
	addresstype() const noexcept
	{
		return *static_cast<const bittype*>(&load_op::addresstype());
	}

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(
		jive::output * address,
		size_t nbits,
		const jive::valuetype & valuetype,
		const std::vector<jive::output*> & states)
	{
		std::vector<jive::output*> operands(1, address);
		operands.insert(operands.end(), states.begin(), states.end());

		jive::bitload_op op(bittype(nbits), valuetype, states.size());
		return simple_node::create_normalized(address->region(), op, operands)[0];
	}
};

}

#endif
