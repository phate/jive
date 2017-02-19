/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_DATAOBJECT_H
#define JIVE_ARCH_DATAOBJECT_H

#include <jive/common.h>

#include <jive/util/ptr-collection.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {

class dataobj_tail_op final : public region_tail_op {
public:
	virtual
	~dataobj_tail_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class dataobj_op final : public region_anchor_op {
public:
	virtual
	~dataobj_op() noexcept;

	inline
	dataobj_op(const std::vector<std::unique_ptr<const base::type>> & types)
	: types_(detail::unique_ptr_vector_copy(types))
	{}

	inline
	dataobj_op(std::vector<std::unique_ptr<const base::type>> && types)
	: types_(std::move(types))
	{}

	inline
	dataobj_op(const dataobj_op & other)
	: types_(detail::unique_ptr_vector_copy(other.types_))
	{}

	inline
	dataobj_op(dataobj_op && other) noexcept = default;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<std::unique_ptr<const base::type>> types_;
};

class memlayout_mapper;

}

jive::oport *
jive_dataobj(jive::oport * data, jive::memlayout_mapper * mapper);

jive::oport *
jive_rodataobj(jive::oport * data, jive::memlayout_mapper * mapper);

jive::oport *
jive_bssobj(jive::oport * data, jive::memlayout_mapper * mapper);

#endif
