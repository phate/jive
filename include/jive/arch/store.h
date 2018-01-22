/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_STORE_H
#define JIVE_ARCH_STORE_H

#include <jive/arch/addresstype.h>
#include <jive/rvsdg/node-normal-form.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/type.h>

namespace jive {

/* store operator */

class store_op : public simple_op {
public:
	virtual
	~store_op() noexcept;

protected:
	inline
	store_op(
		const jive::valuetype & address,
		const jive::valuetype & value,
		size_t nstates)
	: simple_op(create_operands(address, value, nstates),
			std::vector<jive::port>(nstates, {memtype()}))
	{}

public:
	virtual bool
	operator==(const operation & other) const noexcept override final;

	inline const jive::valuetype &
	addresstype() const noexcept
	{
		return *static_cast<const jive::valuetype*>(&argument(0).type());
	}

	inline const jive::valuetype &
	valuetype() const noexcept
	{
		return *static_cast<const jive::valuetype*>(&argument(1).type());
	}

private:
	static std::vector<jive::port>
	create_operands(
		const jive::valuetype & address,
		const jive::valuetype & value,
		size_t nstates);
};

static inline bool
is_store_op(const jive::operation & op)
{
	return dynamic_cast<const jive::store_op*>(&op) != nullptr;
}

static inline bool
is_store_node(const jive::node * node)
{
	return is_opnode<store_op>(node);
}

/* address store operator */

class addrstore_op final : public store_op {
public:
	virtual
	~addrstore_op();

	inline
	addrstore_op(
		const jive::addrtype & address,
		const jive::valuetype & value,
		size_t nstates)
	: store_op(address, value, nstates)
	{}

	inline const jive::addrtype &
	addresstype() const noexcept
	{
		return *static_cast<const addrtype*>(&store_op::addresstype());
	}

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<operation>
	copy() const override;

	static inline std::vector<jive::output*>
	create(
		jive::output * address,
		jive::output * value,
		const jive::valuetype & valuetype,
		const std::vector<jive::output*> & states)
	{
		std::vector<jive::output*> operands({address, value});
		operands.insert(operands.end(), states.begin(), states.end());

		jive::addrstore_op op(jive::addrtype(), valuetype, states.size());
		return simple_node::create_normalized(address->region(), op, operands);
	}
};

static inline bool
is_addrstore_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::addrstore_op*>(&op) != nullptr;
}

static inline bool
is_addrstore_node(const jive::node * node) noexcept
{
	return is_opnode<addrstore_op>(node);
}

/* bitstring store operator */

class bitstore_op final : public store_op {
public:
	virtual
	~bitstore_op();

	inline
	bitstore_op(
		const bittype & address,
		const jive::valuetype & value,
		size_t nstates)
	: store_op(address, value, nstates)
	{}

	inline const bittype &
	addresstype() const noexcept
	{
		return *static_cast<const bittype*>(&store_op::addresstype());
	}

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<operation>
	copy() const override;

	static inline std::vector<jive::output*>
	create(
		jive::output * address,
		jive::output * value,
		size_t nbits,
		const jive::valuetype & valuetype,
		const std::vector<jive::output*> & states)
	{
		std::vector<jive::output*> operands({address, value});
		operands.insert(operands.end(), states.begin(), states.end());

		bitstore_op op(bittype(nbits), valuetype, states.size());
		return simple_node::create_normalized(address->region(), op, operands);
	}
};

static inline bool
is_bitstore_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::bitstore_op*>(&op) != nullptr;
}

static inline bool
is_bitstore_op(const jive::node * node) noexcept
{
	return is_opnode<bitstore_op>(node);
}

}

#endif
