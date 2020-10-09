/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESSTYPE_HPP
#define JIVE_ARCH_ADDRESSTYPE_HPP

#include <jive/rvsdg/type.hpp>

namespace jive {

/* address type */

class addrtype final : public jive::valuetype {
public:
	virtual
	~addrtype() noexcept;

	inline
	addrtype(const valuetype & type)
	: valuetype()
	, type_(type.copy())
	{}

	inline
	addrtype(const addrtype & other)
	: valuetype(other)
	, type_(other.type_->copy())
	{}

	inline
	addrtype(addrtype && other)
	: valuetype(other)
	, type_(std::move(other.type_))
	{}

	addrtype &
	operator=(const addrtype & other)
	{
		if (this != &other)
			type_ = other.type_->copy();

		return *this;
	}

	addrtype &
	operator=(addrtype && other)
	{
		if (this != &other)
			type_ = std::move(other.type_);

		return *this;
	}

	virtual std::string
	debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

	inline const valuetype &
	type() const noexcept
	{
		return *static_cast<const valuetype*>(type_.get());
	}

private:
	std::unique_ptr<jive::type> type_;
};

class memtype final : public jive::statetype {
public:
	virtual ~memtype() noexcept;

	inline constexpr
	memtype() noexcept
	: jive::statetype()
	{}

	virtual std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

	inline static const memtype & instance() { return instance_; }

private:
	static const memtype instance_;
};

}

#endif
