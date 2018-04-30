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
#include <jive/types/function/fcttype.h>

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
	: simple_op(create_operands(address, arguments), create_results(results))
	, callconv_(callconv)
	{}

public:
	inline const jive::valuetype &
	addresstype() const noexcept
	{
		return *static_cast<const jive::valuetype*>(&argument(0).type());
	}

	virtual bool
	operator==(const operation & other) const noexcept override final;

	virtual std::string
	debug_string() const override final;

	inline const jive_calling_convention *
	calling_convention() const noexcept
	{
		return callconv_;
	}

private:
	static std::vector<jive::port>
	create_operands(
		const valuetype & address,
		const std::vector<const type*> & arguments);

	static std::vector<jive::port>
	create_results(const std::vector<const type*> & results);

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
	return is<call_op>(node);
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
	: call_op(addrtype(fct::type(arguments, results)), arguments, results, callconv)
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
		return simple_node::create_normalized(address->region(), op, operands);
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
	return is<addrcall_op>(node);
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
	: call_op(bittype(nbits), arguments, results, callconv)
	{}

	inline const bittype &
	addresstype() const noexcept
	{
		return *static_cast<const bittype*>(&call_op::addresstype());
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
		return simple_node::create_normalized(address->region(), op, operands);
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
	return is<bitcall_op>(node);
}

}

#endif
