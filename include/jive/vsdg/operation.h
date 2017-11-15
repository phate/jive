/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATION_H
#define JIVE_VSDG_OPERATION_H

#include <jive/vsdg/type.h>

#include <memory>
#include <string>
#include <vector>

namespace jive {

class gate;
class node;
class node_normal_form;
class output;
class region;
class resource_class;

/* port */

class port final {
public:
	port(jive::gate * gate);

	port(const jive::type & type);

	port(std::unique_ptr<jive::type> type);

	port(const resource_class * rescls);

	inline
	port(const port & other)
	: gate_(other.gate_)
	, rescls_(other.rescls_)
	, type_(other.type_->copy())
	{}

	inline
	port(port && other)
	: gate_(other.gate_)
	, rescls_(other.rescls_)
	, type_(std::move(other.type_))
	{
		other.gate_ = nullptr;
		other.rescls_ = nullptr;
	}

	inline port &
	operator=(const port & other)
	{
		if (&other == this)
			return *this;

		gate_ = other.gate_;
		rescls_ = other.rescls_;
		type_ = std::move(other.type_->copy());

		return *this;
	}

	inline port &
	operator=(port && other)
	{
		if (&other == this)
			return *this;

		gate_ = other.gate_;
		rescls_ = other.rescls_;
		type_ = std::move(other.type_);
		other.gate_ = nullptr;
		other.rescls_ = nullptr;

		return *this;
	}

	inline bool
	operator==(const port & other) const noexcept
	{
		return gate_ == other.gate_
		    && rescls_ == other.rescls_
		    && *type_ == *other.type_;
	}

	inline bool
	operator!=(const port & other) const noexcept
	{
		return !(*this == other);
	}

	inline jive::gate *
	gate() const noexcept
	{
		return gate_;
	}

	inline const resource_class *
	rescls() const noexcept
	{
		return rescls_;
	}

	inline const jive::type &
	type() const noexcept
	{
		return *type_;
	}

private:
	jive::gate * gate_;
	const resource_class * rescls_;
	std::unique_ptr<jive::type> type_;
};

/* operation */

class operation {
public:
	virtual ~operation() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept = 0;

	virtual size_t
	narguments() const noexcept = 0;

	virtual const jive::port &
	argument(size_t index) const noexcept = 0;

	virtual size_t
	nresults() const noexcept = 0;

	virtual const jive::port &
	result(size_t index) const noexcept = 0;

	virtual std::string
	debug_string() const = 0;

	virtual std::unique_ptr<jive::operation>
	copy() const = 0;

	inline bool
	operator!=(const operation & other) const noexcept
	{
		return ! (*this == other);
	}
};

/* simple operation */

class simple_op : public operation {
public:
	virtual
	~simple_op();
};

/* structural operation */

class structural_op : public operation {
public:
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
};

}

#endif
