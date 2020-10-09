/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_EVALUATOR_LITERAL_HPP
#define JIVE_EVALUATOR_LITERAL_HPP

#include <jive/rvsdg/control.hpp>
#include <jive/types/bitstring/type.hpp>
#include <jive/types/bitstring/value-representation.hpp>
#include <jive/types/function.hpp>

#include <vector>

namespace jive {
namespace eval {

class literal {
public:
	virtual
	~literal() noexcept;

protected:
	inline constexpr
	literal() noexcept
	{}

public:
	virtual const jive::type &
	type() const noexcept = 0;

	virtual std::unique_ptr<literal>
	copy() const = 0;
};

class bitliteral final : public literal {
public:
	virtual
	~bitliteral() noexcept;

	inline
	bitliteral(const bitvalue_repr & vr)
		: vr_(vr)
		, type_(vr.nbits())
	{}

	virtual const jive::type &
	type() const noexcept override;

	virtual std::unique_ptr<literal>
	copy() const override;

	inline const bitvalue_repr &
	value_repr() const noexcept
	{
		return vr_;
	}

private:
	bitvalue_repr vr_;
	bittype type_;
};

class ctlliteral final : public literal {
public:
	virtual
	~ctlliteral() noexcept;

	inline
	ctlliteral(const jive::ctlvalue_repr & vr)
		: vr_(vr)
		, type_(vr.nalternatives())
	{}

	virtual const jive::type &
	type() const noexcept override;

	virtual std::unique_ptr<literal>
	copy() const override;

	inline const jive::ctlvalue_repr &
	value_repr() const noexcept
	{
		return vr_;
	}

	inline size_t
	alternative() const noexcept
	{
		return vr_.alternative();
	}

private:
	jive::ctlvalue_repr vr_;
	jive::ctltype type_;
};

class fctliteral final : public literal {
public:
	virtual
	~fctliteral() noexcept;

	fctliteral(
		const std::vector<std::unique_ptr<const literal>> & arguments,
		const std::vector<std::unique_ptr<const literal>> & result);

	fctliteral(
		const std::vector<const literal*> & arguments,
		const std::vector<const literal*> & results);

	fctliteral(const fctliteral & other);

	fctliteral &
	operator=(const fctliteral & other);

	virtual const jive::type &
	type() const noexcept override;

	virtual std::unique_ptr<literal>
	copy() const override;

	inline size_t
	narguments() const noexcept
	{
		return arguments_.size();
	}

	inline const literal &
	argument(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < narguments());
		return *arguments_[index];
	}

	inline size_t
	nresults() const noexcept
	{
		return results_.size();
	}

	inline const literal &
	result(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nresults());
		return *results_[index];
	}

private:
	std::unique_ptr<jive::type> type_;
	std::vector<std::unique_ptr<const literal>> arguments_;
	std::vector<std::unique_ptr<const literal>> results_;
};

class memliteral final : public literal {
public:
	virtual
	~memliteral() noexcept;

	virtual const jive::type &
	type() const noexcept override;

	virtual std::unique_ptr<literal>
	copy() const override;
};

}
}

#endif
