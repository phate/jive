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
#include <jive/types/record.h>

namespace jive {

class label;

/* "memberof" operator: given an address that is the start of a record
in memory, compute address of specified member of record */

class memberof_op : public jive::unary_op {
public:
	virtual ~memberof_op() noexcept;

	inline
	memberof_op(
		const jive::rcddeclaration * dcl,
		size_t index)
	: unary_op(addrtype::instance(), addrtype::instance())
	, index_(index)
	, dcl_(dcl)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline const jive::rcddeclaration *
	record_decl() const noexcept
	{
		return dcl_;
	}

	inline size_t
	index() const noexcept { return index_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	size_t index_;
	const jive::rcddeclaration * dcl_;
};

class containerof_op : public jive::unary_op {
public:
	virtual ~containerof_op() noexcept;

	inline
	containerof_op(
		const jive::rcddeclaration * dcl,
		size_t index)
	: unary_op(addrtype::instance(), addrtype::instance())
	, index_(index)
	, dcl_(dcl)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline const jive::rcddeclaration *
	record_decl() const noexcept
	{
		return dcl_;
	}

	inline size_t
	index() const noexcept { return index_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	size_t index_;
	const rcddeclaration * dcl_;
};

class arraysubscript_op : public simple_op {
public:
	virtual
	~arraysubscript_op() noexcept;

	inline
	arraysubscript_op(
		const jive::valuetype & element_type,
		const bittype & index_type)
	: simple_op({addrtype::instance(), index_type}, {addrtype::instance()})
	, element_type_(std::move(element_type.copy()))
	{}

	inline
	arraysubscript_op(const arraysubscript_op & other)
	: simple_op(other)
	, element_type_(other.element_type_->copy())
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::valuetype &
	element_type() const noexcept
	{
		return *static_cast<const valuetype*>(element_type_.get());
	}

	inline const bittype &
	index_type() const noexcept
	{
		return *static_cast<const bittype*>(&argument(1).type());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<jive::type> element_type_;
};

class arrayindex_op : public simple_op {
public:
	virtual
	~arrayindex_op() noexcept;

	inline
	arrayindex_op(
		const valuetype & element_type,
		const bittype & index_type)
	: simple_op({addrtype::instance(), addrtype::instance()}, {index_type})
	, element_type_(element_type.copy())
	{}

	inline
	arrayindex_op(const arrayindex_op & other)
	: simple_op(other)
	, element_type_(other.element_type_->copy())
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::valuetype &
	element_type() const noexcept
	{
		return *static_cast<const valuetype*>(element_type_.get());
	}

	inline const bittype &
	index_type() const noexcept
	{
		return *static_cast<const bittype*>(&result(0).type());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<jive::type> element_type_;
};

/* lbl2addr operation */

class lbl2addr_op final : public nullary_op {
public:
	virtual
	~lbl2addr_op() noexcept;

	inline
	lbl2addr_op(const jive::label * label) noexcept
	: nullary_op(addrtype::instance())
	, label_(label)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	const jive::label *
	label() const noexcept
	{
		return label_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::region * region, const jive::label * lbl)
	{
		lbl2addr_op op(lbl);
		return simple_node::create_normalized(region, op, {})[0];
	}

private:
	const jive::label * label_;
};

static inline bool
is_lbl2addr_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::lbl2addr_op*>(&op) != nullptr;
}

static inline bool
is_lbl2addr_node(const jive::node * node) noexcept
{
	return is_opnode<lbl2addr_op>(node);
}

/* label to bistring operation */

class lbl2bit_op final : public nullary_op {
public:
	virtual
	~lbl2bit_op() noexcept;

	inline
	lbl2bit_op(
		const jive::label * label,
		size_t nbits) noexcept
	: nullary_op(bittype(nbits))
	, label_(label)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

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
		return static_cast<const bittype*>(&result(0).type())->nbits();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::region * region, size_t nbits, const jive::label * lbl)
	{
		lbl2bit_op op(lbl, nbits);
		return simple_node::create_normalized(region, op, {})[0];
	}

private:
	const jive::label * label_;
};

static inline bool
is_lbl2bit_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const lbl2bit_op*>(&op) != nullptr;
}

static inline bool
is_lbl2bit_node(const jive::node * node) noexcept
{
	return is_opnode<lbl2bit_op>(node);
}

/* address value representation */

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

struct addrtype_of_value {
	addrtype
	operator()(const value_repr & vr) const
	{
		return jive::addrtype();
	}
};

struct addrformat_value {
	std::string
	operator()(const value_repr & vr) const
	{
		return vr.debug_string();
	}
};

typedef domain_const_op<addrtype, value_repr, addrformat_value, addrtype_of_value> addrconstant_op;

output *
constant(jive::graph * graph, const value_repr & vr);

}

jive::output *
jive_memberof(
	jive::output * address,
	const jive::rcddeclaration * dcl,
	size_t index);

/* "containerof" operator: given an address that is the start of a record
member in memory, compute address of containing record */

jive::output *
jive_containerof(
	jive::output * address,
	const jive::rcddeclaration * dcl,
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
	const jive::bittype * difference_type);

#endif
