/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_TYPE_H
#define JIVE_ARCH_IMMEDIATE_TYPE_H

#include <jive/vsdg/valuetype.h>

typedef struct jive_immediate_type jive_immediate_type;

extern const jive_type_class JIVE_IMMEDIATE_TYPE;
class jive_immediate_type final : public jive_value_type {
public:
	virtual ~jive_immediate_type() noexcept;

	jive_immediate_type() noexcept;

	virtual void label(jive_buffer & buffer) const override;

	virtual bool operator==(const jive_type & other) const noexcept override;

	virtual std::unique_ptr<jive_type> copy() const override;

	virtual jive_input * create_input(jive_node * node, size_t index,
		jive_output * origin) const override;

	virtual jive_output * create_output(jive_node * node, size_t index) const override;

	virtual jive_gate * create_gate(jive_graph * graph, const char * name) const override;
};

class jive_immediate_input final : public jive_value_input {
public:
	virtual ~jive_immediate_input() noexcept;

	jive_immediate_input(struct jive_node * node, size_t index, jive_output * origin);

	virtual const jive_immediate_type & type() const noexcept { return type_; }

private:
	jive_immediate_type type_;
};

class jive_immediate_output final : public jive_value_output {
public:
	virtual ~jive_immediate_output() noexcept;

	jive_immediate_output(jive_node * node, size_t index);

	virtual const jive_immediate_type & type() const noexcept { return type_; }

private:
	jive_immediate_type type_;
};

class jive_immediate_gate final : public jive_value_gate {
public:
	virtual ~jive_immediate_gate() noexcept;

	jive_immediate_gate(jive_graph * graph, const char name[]);

	virtual const jive_immediate_type & type() const noexcept { return type_; }

private:
	jive_immediate_type type_;
};

#endif
