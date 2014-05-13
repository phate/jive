/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHORTYPE_H
#define JIVE_VSDG_ANCHORTYPE_H

#include <jive/vsdg/basetype.h>

typedef struct jive_anchor_type jive_anchor_type;

extern const jive_type_class JIVE_ANCHOR_TYPE;
class jive_anchor_type final : public jive_type {
public:
	virtual ~jive_anchor_type() noexcept;

	jive_anchor_type() noexcept;

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual std::unique_ptr<jive_type> copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;
};

class jive_anchor_input final : public jive_input {
public:
	virtual ~jive_anchor_input() noexcept;

	jive_anchor_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_anchor_type & type() const noexcept { return type_; }

private:
	jive_anchor_type type_;
};

class jive_anchor_output final : public jive_output {
public:
	virtual ~jive_anchor_output() noexcept;

	jive_anchor_output(struct jive_node * node, size_t index);

	virtual const jive_anchor_type & type() const noexcept { return type_; }

private:
	jive_anchor_type type_;
};

#endif
