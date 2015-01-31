/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTTYPE_H
#define JIVE_TYPES_FUNCTION_FCTTYPE_H

#include <jive/vsdg/valuetype.h>

#include <memory>
#include <vector>

namespace jive {
namespace fct {

/* function type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	type(size_t narguments, const jive::base::type ** argument_types, size_t nreturns,
		const jive::base::type ** return_types);

	type(const std::vector<std::unique_ptr<jive::base::type>> & argument_types,
		const std::vector<std::unique_ptr<jive::base::type>> & return_types);

	type(const jive::fct::type & rhs);

	type(jive::fct::type && other) noexcept;

	inline size_t nreturns() const noexcept { return return_types_.size(); }

	inline size_t narguments() const noexcept { return argument_types_.size(); }

	inline const jive::base::type * return_type(size_t index) const noexcept
		{ return return_types_[index].get(); }

	inline const jive::base::type * argument_type(size_t index) const noexcept
		{ return argument_types_[index].get(); }

	inline const std::vector<std::unique_ptr<jive::base::type>> &
	return_types() const noexcept { return return_types_; }

	inline const std::vector<std::unique_ptr<jive::base::type>> &
	argument_types() const noexcept { return argument_types_; }

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual jive::fct::type * copy() const override;

	virtual jive::output * create_output(jive_node * node, size_t index) const override;

	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const override;

	jive::fct::type& operator=(const jive::fct::type & rhs);

private:
	std::vector<std::unique_ptr<jive::base::type>> return_types_;
	std::vector<std::unique_ptr<jive::base::type>> argument_types_;
};

/* function output */

class output final : public jive::value::output {
public:
	virtual ~output() noexcept;

	output(const jive::fct::type & type, jive_node * node, size_t index);

	output(size_t narguments, const jive::base::type ** argument_types, size_t nreturns,
		const jive::base::type ** return_types, jive_node * node, size_t index);

	virtual const jive::fct::type & type() const noexcept { return type_; }

	inline size_t narguments() const noexcept { return type_.narguments(); }

	inline size_t nreturns() const noexcept { return type_.nreturns(); }

	inline const jive::base::type * argument_type(size_t index) const noexcept
		{ return type_.argument_type(index); }

	inline const jive::base::type * return_type(size_t index) const noexcept
		{ return type_.return_type(index); }

private:
	output(const output & rhs) = delete;
	output& operator=(const output & rhs) = delete;

	jive::fct::type type_;
};

/* function gate */

class gate final : public jive::value::gate {
public:
	virtual ~gate() noexcept;

	gate(const jive::fct::type & type, jive_graph * graph, const char name[]);

	gate(size_t narguments, const jive::base::type ** argument_types, size_t nreturns,
		const jive::base::type ** return_types, jive_graph * graph, const char name[]);

	virtual const jive::fct::type & type() const noexcept { return type_; }

	inline size_t narguments() const noexcept { return type_.narguments(); }

	inline size_t nreturns() const noexcept { return type_.nreturns(); }

	inline const jive::base::type * argument_type(size_t index) const noexcept
		{ return type_.argument_type(index); }

	inline const jive::base::type * return_type(size_t index) const noexcept
		{ return type_.return_type(index); }

private:
	gate(const gate & rhs) = delete;
	gate& operator=(const gate & rhs) = delete;

	jive::fct::type type_;
};

}
}

#endif
