/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESS_H
#define JIVE_ARCH_ADDRESS_H

#include <memory>

#include <jive/arch/addresstype.h>
#include <jive/common.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/unary.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdtype.h>

/* "memberof" operator: given an address that is the start of a record
in memory, compute address of specified member of record */

namespace jive {

class label;

namespace address {

class memberof_op : public jive::unary_op {
public:
	virtual ~memberof_op() noexcept;

	inline
	memberof_op(
		std::shared_ptr<const jive::rcd::declaration> & decl,
		size_t index)
		: record_decl_(decl),
		index_(index)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline const std::shared_ptr<const jive::rcd::declaration> &
	record_decl() const noexcept
	{
		return record_decl_;
	}

	inline size_t
	index() const noexcept { return index_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::shared_ptr<const jive::rcd::declaration> record_decl_;
	size_t index_;
};

class containerof_op : public jive::unary_op {
public:
	virtual ~containerof_op() noexcept;

	inline
	containerof_op(
		std::shared_ptr<const jive::rcd::declaration> & decl,
		size_t index)
		: record_decl_(decl),
		index_(index)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline const std::shared_ptr<const jive::rcd::declaration> &
	record_decl() const noexcept
	{
		return record_decl_;
	}

	inline size_t
	index() const noexcept { return index_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::shared_ptr<const jive::rcd::declaration> record_decl_;
	size_t index_;
};

class arraysubscript_op : public simple_op {
public:
	virtual ~arraysubscript_op() noexcept;

	arraysubscript_op(const arraysubscript_op & other);
	arraysubscript_op(arraysubscript_op && other) noexcept;
	arraysubscript_op(
		const jive::valuetype & element_type,
		const jive::bits::type & index_type);

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::valuetype &
	element_type() const noexcept
	{
		return *static_cast<const valuetype*>(element_type_.get());
	}

	inline const jive::bits::type &
	index_type() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&index_.type());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port index_;
	std::unique_ptr<jive::type> element_type_;
};

class arrayindex_op : public simple_op {
public:
	virtual ~arrayindex_op() noexcept;

	arrayindex_op(const arrayindex_op & other);
	arrayindex_op(arrayindex_op && other) noexcept;
	arrayindex_op(
		const jive::valuetype & element_type,
		const jive::bits::type & index_type);

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::valuetype &
	element_type() const noexcept
	{
		return *static_cast<const valuetype*>(element_type_.get());
	}

	inline const jive::bits::type &
	index_type() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&index_.type());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port index_;
	std::unique_ptr<jive::type> element_type_;
};

class label_to_address_op : public base::nullary_op {
public:
	virtual
	~label_to_address_op() noexcept;

	inline constexpr
	label_to_address_op(const jive::label * label) noexcept
		: label_(label)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	const jive::label *
	label() const noexcept
	{
		return label_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::label * label_;
};

class label_to_bitstring_op : public base::nullary_op {
public:
	virtual
	~label_to_bitstring_op() noexcept;

	inline
	label_to_bitstring_op(
		const jive::label * label,
		size_t nbits) noexcept
	: base::nullary_op()
	, result_(jive::bits::type(nbits))
	, label_(label)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	const jive::label *
	label() const noexcept
	{
		return label_;
	}

	size_t
	nbits() const noexcept
	{
		return static_cast<const jive::bits::type*>(&result_.type())->nbits();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port result_;
	const jive::label * label_;
};

class value_repr final {
public:
	/*
		FIXME: This should take a bitstring of arbitrary length.
	*/
	inline constexpr
	value_repr(uint64_t address) noexcept
		: address_(address)
	{}

	inline std::string
	debug_string() const
	{
		return detail::strfmt(address_);
	}

	inline bool
	operator==(const value_repr & other) const noexcept
	{
		return address_ == other.address_;
	}

	inline bool
	operator!=(const value_repr & other) const noexcept
	{
		return !(*this == other);
	}

	inline uint64_t
	value() const noexcept
	{
		return address_;
	}

private:
	uint64_t address_;
};

struct type_of_value {
	addrtype operator()(const value_repr & vr) const
	{
		return jive::addrtype();
	}
};

struct format_value {
	std::string operator()(const value_repr & vr) const
	{
		return vr.debug_string();
	}
};

typedef base::domain_const_op<
	addrtype, value_repr, format_value, type_of_value
> constant_op;

output *
constant(jive::graph * graph, const value_repr & vr);

}
}

jive::output *
jive_memberof(
	jive::output * address,
	std::shared_ptr<const jive::rcd::declaration> & record_decl,
	size_t index);

/* "containerof" operator: given an address that is the start of a record
member in memory, compute address of containing record */

jive::output *
jive_containerof(
	jive::output * address,
	std::shared_ptr<const jive::rcd::declaration> & record_decl,
	size_t index);

/* "arraysubscript" operator: given an address that points to an element of
an array, compute address of element offset by specified distance */

jive::output *
jive_arraysubscript(jive::output * address, const jive::valuetype * element_type,
	jive::output * index);

/* "arrayindex" operator: given two addresses that each point to an
element of an array and the array element type, compute the
difference of their indices */

jive::output *
jive_arrayindex(jive::output * addr1, jive::output * addr2,
	const jive::valuetype * element_type,
	const jive::bits::type * difference_type);

/* label_to_address node */

jive::output *
jive_label_to_address_create(jive::region * region, const jive::label * label);

/* label_to_bitstring node */

jive::output *
jive_label_to_bitstring_create(jive::region * region, const jive::label * label, size_t nbits);

#endif
