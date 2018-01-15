/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CALL_H
#define JIVE_ARCH_CALL_H

#include <jive/arch/addresstype.h>
#include <jive/common.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/type.h>

namespace jive {
	class type;
}

/* FIXME: opaque type for now -- to be filled in later */
struct jive_calling_convention;

namespace jive {

/* call operation */

class call_op : public simple_op {
public:
	virtual
	~call_op() noexcept;

protected:
	inline
	call_op(
		const jive::valuetype & address,
		const std::vector<const jive::type*> & arguments,
		const std::vector<const jive::type*> & results,
		const jive_calling_convention * callconv)
	: address_(address)
	, callconv_(callconv)
	{
		for (const auto & type : arguments)
			arguments_.push_back({std::move(type->copy())});
		for (const auto & type : results)
			results_.push_back({std::move(type->copy())});
	}

	call_op(const call_op &) = default;

	call_op(call_op &&) = default;

public:
	inline const jive::valuetype &
	addresstype() const noexcept
	{
		return *static_cast<const jive::valuetype*>(&address_.type());
	}

	virtual bool
	operator==(const operation & other) const noexcept override final;

	virtual size_t
	narguments() const noexcept override final;

	virtual const jive::port &
	argument(size_t index) const noexcept override final;

	virtual size_t
	nresults() const noexcept override final;

	virtual const jive::port &
	result(size_t index) const noexcept override final;

	virtual std::string
	debug_string() const override final;

	inline const jive_calling_convention *
	calling_convention() const noexcept
	{
		return callconv_;
	}

private:
	jive::port address_;
	std::vector<jive::port> results_;
	std::vector<jive::port> arguments_;
	const jive_calling_convention * callconv_;
};

static inline bool
is_call_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const call_op*>(&op) != nullptr;
}

static inline bool
is_call_node(const jive::node * node) noexcept
{
	return is_opnode<call_op>(node);
}

/* address call operation */

class addrcall_op final : public call_op {
public:
	virtual
	~addrcall_op();

	inline
	addrcall_op(
		const std::vector<const jive::type*> & arguments,
		const std::vector<const jive::type*> & results,
		const jive_calling_convention * callconv)
	: call_op(jive::addrtype(), arguments, results, callconv)
	{}

	inline const jive::addrtype &
	addresstype() const noexcept
	{
		return *static_cast<const jive::addrtype*>(&call_op::addresstype());
	}

	static inline std::vector<jive::output*>
	create(
		jive::output * address,
		const std::vector<jive::output*> & arguments,
		const std::vector<const jive::type*> & resulttypes,
		const jive_calling_convention * callconv)
	{
		std::vector<const jive::type*> argumenttypes;
		std::vector<jive::output*> operands({address});
		for (const auto & argument : arguments) {
			operands.push_back(argument);
			argumenttypes.push_back(&argument->type());
		}

		addrcall_op op(argumenttypes, resulttypes, callconv);
		return jive::create_normalized(address->region(), op, operands);
	}

	virtual std::unique_ptr<operation>
	copy() const override;
};

static inline bool
is_addrcall_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const addrcall_op*>(&op) != nullptr;
}

static inline bool
is_addrcall_node(const jive::node * node) noexcept
{
	return is_opnode<addrcall_op>(node);
}

/* bitstring call operation */

class bitcall_op final : public call_op {
public:
	virtual
	~bitcall_op();

	inline
	bitcall_op(
		size_t nbits,
		const std::vector<const jive::type*> & arguments,
		const std::vector<const jive::type*> & results,
		const jive_calling_convention * callconv)
	: call_op(jive::bits::type(nbits), arguments, results, callconv)
	{}

	inline const jive::bits::type &
	addresstype() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&call_op::addresstype());
	}

	static inline std::vector<jive::output*>
	create(
		jive::output * address,
		size_t nbits,
		const std::vector<jive::output*> & arguments,
		const std::vector<const jive::type*> & resulttypes,
		const jive_calling_convention * callconv)
	{
		std::vector<const jive::type*> argumenttypes;
		std::vector<jive::output*> operands({address});
		for (const auto & argument : arguments) {
			operands.push_back(argument);
			argumenttypes.push_back(&argument->type());
		}

		bitcall_op op(nbits, argumenttypes, resulttypes, callconv);
		return jive::create_normalized(address->region(), op, operands);
	}

	virtual std::unique_ptr<operation>
	copy() const override;
};

static inline bool
is_bitcall_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::bitcall_op*>(&op) != nullptr;
}

static inline bool
is_bitcall_node(const jive::node * node) noexcept
{
	return is_opnode<bitcall_op>(node);
}

}

#endif
