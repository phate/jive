/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTTYPE_H
#define JIVE_TYPES_FUNCTION_FCTTYPE_H

#include <jive/vsdg/type.h>

#include <memory>
#include <vector>

namespace jive {
namespace fct {

class type final : public jive::valuetype {
public:
	virtual
	~type() noexcept;

	type(
		const std::vector<const jive::type*> & argument_types,
		const std::vector<const jive::type*> & result_types);

	type(
		const std::vector<std::unique_ptr<jive::type>> & argument_types,
		const std::vector<std::unique_ptr<jive::type>> & result_types);

	type(const jive::fct::type & other);

	inline size_t
	nresults() const noexcept
	{
		return result_types_.size();
	}

	inline size_t
	narguments() const noexcept
	{
		return argument_types_.size();
	}

	inline const jive::type &
	result_type(size_t index) const noexcept
	{
		return *result_types_[index];
	}

	inline const jive::type &
	argument_type(size_t index) const noexcept
	{
		return *argument_types_[index];
	}

	virtual std::string
	debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

	jive::fct::type &
	operator=(const jive::fct::type & other);

private:
	std::vector<std::unique_ptr<jive::type>> result_types_;
	std::vector<std::unique_ptr<jive::type>> argument_types_;
};

}
}

#endif
