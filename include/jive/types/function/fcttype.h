/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTTYPE_H
#define JIVE_TYPES_FUNCTION_FCTTYPE_H

#include <jive/vsdg/basetype.h>

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

	jive::fct::type& operator=(const jive::fct::type & rhs);

private:
	std::vector<std::unique_ptr<jive::base::type>> return_types_;
	std::vector<std::unique_ptr<jive::base::type>> argument_types_;
};

}
}

#endif
