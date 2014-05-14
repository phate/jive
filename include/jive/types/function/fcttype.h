/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTTYPE_H
#define JIVE_TYPES_FUNCTION_FCTTYPE_H

#include <jive/vsdg/valuetype.h>

#include <memory>
#include <vector>

/* function type */

extern const jive_type_class JIVE_FUNCTION_TYPE;

typedef struct jive_function_type jive_function_type;
class jive_function_type final : public jive_value_type {
public:
	virtual ~jive_function_type() noexcept;

	jive_function_type(size_t narguments, const jive_type ** argument_types,
		size_t nreturns, const jive_type ** return_types);

	jive_function_type(const std::vector<std::unique_ptr<jive_type>> & argument_types,
		const std::vector<std::unique_ptr<jive_type>> & return_types);

	jive_function_type(const jive_function_type & rhs);

	jive_function_type(jive_function_type && other) noexcept;

	inline size_t nreturns() const noexcept { return return_types_.size(); }

	inline size_t narguments() const noexcept { return argument_types_.size(); }

	inline const jive_type * return_type(size_t index) const noexcept
		{ return return_types_[index].get(); }

	inline const jive_type * argument_type(size_t index) const noexcept
		{ return argument_types_[index].get(); }

	inline const std::vector<std::unique_ptr<jive_type>> &
	return_types() const noexcept { return return_types_; }

	inline const std::vector<std::unique_ptr<jive_type>> &
	argument_types() const noexcept { return argument_types_; }

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual std::unique_ptr<jive_type> copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

private:
	std::vector<std::unique_ptr<jive_type>> return_types_;
	std::vector<std::unique_ptr<jive_type>> argument_types_;
};

JIVE_EXPORTED_INLINE struct jive_function_type *
jive_function_type_cast(struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_FUNCTION_TYPE))
		return (jive_function_type *) type;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_function_type *
jive_function_type_const_cast(const struct jive_type * type)
{
	if (jive_type_isinstance(type, &JIVE_FUNCTION_TYPE))
		return (const jive_function_type *) type;
	else
		return NULL;
}

/* function input */

class jive_function_input final : public jive_value_input {
public:
	virtual ~jive_function_input() noexcept;

	jive_function_input(const jive_function_type & type, jive_node * node, size_t index,
		jive_output * origin);

	jive_function_input(size_t narguments, const jive_type ** argument_types, size_t nreturns,
		const jive_type ** return_types, struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_function_type & type() const noexcept { return type_; }

	inline size_t narguments() const noexcept { return type_.narguments(); }

	inline size_t nreturns() const noexcept { return type_.nreturns(); }

	inline const jive_type * argument_type(size_t index) const noexcept
		{ return type_.argument_type(index); }

	inline const jive_type * return_type(size_t index) const noexcept
		{ return type_.return_type(index); }

private:
	jive_function_type type_;
};

/* function output */

class jive_function_output final : public jive_value_output {
public:
	virtual ~jive_function_output() noexcept;

	jive_function_output(const jive_function_type & type, jive_node * node, size_t index);

	jive_function_output(size_t narguments, const jive_type ** argument_types, size_t nreturns,
		const jive_type ** return_types, jive_node * node, size_t index);

	virtual const jive_function_type & type() const noexcept { return type_; }

	inline size_t narguments() const noexcept { return type_.narguments(); }

	inline size_t nreturns() const noexcept { return type_.nreturns(); }

	inline const jive_type * argument_type(size_t index) const noexcept
		{ return type_.argument_type(index); }

	inline const jive_type * return_type(size_t index) const noexcept
		{ return type_.return_type(index); }

private:
	jive_function_type type_;
};

/* function gate */

class jive_function_gate final : public jive_value_gate {
public:
	virtual ~jive_function_gate() noexcept;

	jive_function_gate(const jive_function_type & type, jive_graph * graph, const char name[]);

	jive_function_gate(size_t narguments, const jive_type ** argument_types, size_t nreturns,
		const jive_type ** return_types, jive_graph * graph, const char name[]);

	virtual const jive_function_type & type() const noexcept { return type_; }

	inline size_t narguments() const noexcept { return type_.narguments(); }

	inline size_t nreturns() const noexcept { return type_.nreturns(); }

	inline const jive_type * argument_type(size_t index) const noexcept
		{ return type_.argument_type(index); }

	inline const jive_type * return_type(size_t index) const noexcept
		{ return type_.return_type(index); }

private:
	jive_function_type type_;
};

#endif
