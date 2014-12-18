/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESS_H
#define JIVE_ARCH_ADDRESS_H

#include <memory>

#include <jive/arch/addresstype.h>
#include <jive/common.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdtype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

struct jive_label;

/* "memberof" operator: given an address that is the start of a record
in memory, compute address of specified member of record */

namespace jive {
namespace address {

class memberof_op : public base::unary_op {
public:
	virtual ~memberof_op() noexcept;

	inline constexpr
	memberof_op(
		const jive::rcd::declaration * record_decl,
		size_t index)
		: record_decl_(record_decl),
		index_(index)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline const jive::rcd::declaration *
	record_decl() const noexcept { return record_decl_; }

	inline size_t
	index() const noexcept { return index_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::rcd::declaration * record_decl_;
	size_t index_;
};

class containerof_op : public base::unary_op {
public:
	virtual ~containerof_op() noexcept;

	inline constexpr
	containerof_op(
		const jive::rcd::declaration * record_decl,
		size_t index)
		: record_decl_(record_decl),
		index_(index)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline const jive::rcd::declaration *
	record_decl() const noexcept { return record_decl_; }

	inline size_t
	index() const noexcept { return index_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::rcd::declaration * record_decl_;
	size_t index_;
};

class arraysubscript_op : public operation {
public:
	virtual ~arraysubscript_op() noexcept;

	arraysubscript_op(const arraysubscript_op & other);
	arraysubscript_op(arraysubscript_op && other) noexcept;
	arraysubscript_op(
		const jive::value::type & element_type,
		const jive::bits::type & index_type);

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
	virtual std::string
	debug_string() const override;

	inline const jive::value::type &
	element_type() const noexcept { return *element_type_; }

	inline const jive::bits::type &
	index_type() const noexcept { return index_type_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<jive::value::type> element_type_;
	jive::bits::type index_type_;
};

class arrayindex_op : public operation {
public:
	virtual ~arrayindex_op() noexcept;

	arrayindex_op(const arrayindex_op & other);
	arrayindex_op(arrayindex_op && other) noexcept;
	arrayindex_op(
		const jive::value::type & element_type,
		const jive::bits::type & index_type);

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
	virtual std::string
	debug_string() const override;

	inline const jive::value::type &
	element_type() const noexcept { return *element_type_; }

	inline const jive::bits::type &
	index_type() const noexcept { return index_type_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<jive::value::type> element_type_;
	jive::bits::type index_type_;
};

class label_to_address_op : public base::nullary_op {
public:
	virtual
	~label_to_address_op() noexcept;

	inline constexpr
	label_to_address_op(const jive_label * label) noexcept
		: label_(label)
	{
	}

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
	virtual std::string
	debug_string() const override;

	const jive_label *
	label() const noexcept { return label_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const struct jive_label * label_;
};

class label_to_bitstring_op : public base::nullary_op {
public:
	virtual
	~label_to_bitstring_op() noexcept;

	inline constexpr
	label_to_bitstring_op(
		const jive_label * label,
		size_t nbits) noexcept
		: label_(label)
		, result_type_(nbits)
	{
	}

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
	virtual std::string
	debug_string() const override;

	const jive_label *
	label() const noexcept { return label_; }

	size_t
	nbits() const noexcept { return result_type_.nbits(); }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive_label * label_;
	jive::bits::type result_type_;
};

}
}

jive::output *
jive_memberof(jive::output * address, const jive::rcd::declaration * record_decl, size_t index);

/* "containerof" operator: given an address that is the start of a record
member in memory, compute address of containing record */

jive::output *
jive_containerof(jive::output * address, const jive::rcd::declaration * record_decl, size_t index);

/* "arraysubscript" operator: given an address that points to an element of
an array, compute address of element offset by specified distance */

jive::output *
jive_arraysubscript(jive::output * address, const jive::value::type * element_type,
	jive::output * index);

/* "arrayindex" operator: given two addresses that each point to an
element of an array and the array element type, compute the
difference of their indices */

jive::output *
jive_arrayindex(jive::output * addr1, jive::output * addr2,
	const jive::value::type * element_type,
	const jive::bits::type * difference_type);

/* label_to_address node */

jive::output *
jive_label_to_address_create(jive_graph * graph, const jive_label * label);

/* label_to_bitstring node */

jive::output *
jive_label_to_bitstring_create(
	jive_graph * graph, const jive_label * label, size_t nbits);

#endif
