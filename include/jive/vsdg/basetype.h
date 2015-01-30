/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_BASETYPE_H
#define JIVE_VSDG_BASETYPE_H

#include <memory>
#include <stdbool.h>
#include <stdlib.h>

#include <jive/common.h>
#include <jive/vsdg/gate-interference.h>

namespace jive {
	class gate;
	class input;
	class output;
}

struct jive_buffer;
struct jive_cpureg;
struct jive_graph;
struct jive_node;
struct jive_regcls;
struct jive_region;
struct jive_region_hull_entry;
struct jive_resource_class;
struct jive_ssavar;
struct jive_variable;

namespace jive {
namespace base {

/**
        \defgroup jive_type Types
        Types
        @{
*/

class type {
public:
	virtual ~type() noexcept;

protected:
	inline constexpr type() noexcept {};

public:
	virtual bool operator==(const jive::base::type & other) const noexcept = 0;

	inline bool operator!=(const jive::base::type & other) const noexcept { return !(*this == other); }

	virtual jive::base::type * copy() const = 0;

	virtual std::string debug_string() const = 0;

	/*
		FIXME: change return type to std::unique_ptr<jive::input>
	*/
	virtual jive::input * create_input(jive_node * node, size_t index,
		jive::output * origin) const = 0;

	/*
		FIXME: change return type to std::unique_ptr<jive::output>
	*/
	virtual jive::output * create_output(jive_node * node, size_t index) const = 0;

	/*
		FIXME: change return type to std::unique_ptr<jive::gate>
	*/
	virtual jive::gate * create_gate(jive_graph * graph, const char * name) const = 0;
};

}
}	//jive namespace

jive::input *
jive_type_create_input(const jive::base::type * self, struct jive_node * node, size_t index,
	jive::output * origin);

namespace jive {

/**
        \defgroup jive::input Inputs
        Inputs
        @{
*/

class input {
public:
	virtual ~input() noexcept;

protected:
	input(
		struct jive_node * node,
		size_t index,
		jive::output * origin,
		const jive::base::type & type);

public:
	inline const jive::base::type &
	type() const noexcept
	{
		return *type_;
	}

	virtual void label(jive_buffer & buffer) const;

	/*
		FIXME: Try to merge internal_divert_origin and divert_origin methods.
	*/
	void internal_divert_origin(jive::output * new_origin) noexcept;

	void divert_origin(jive::output * new_origin) noexcept;
	
	/*
		FIXME: This function is only used two times in src/regalloc/fixup.c. See whether we can
		actually remove it and add a replacement in the register allocator.
	*/
	void swap(input * other) noexcept;

	inline jive::output * origin() const noexcept { return origin_; }

	inline jive_node * producer() const noexcept;

	struct jive_node * node;
	size_t index;

	struct {
		input * prev;
		input * next;
	} output_users_list;
	
	jive::gate * gate;
	struct {
		input * prev;
		input * next;
	} gate_inputs_list;
	
	struct jive_ssavar * ssavar;
	struct {
		input * prev;
		input * next;
	} ssavar_input_list;

	struct {
		struct jive_region_hull_entry * first;
		struct jive_region_hull_entry * last;
	} hull;
	
	const struct jive_resource_class * required_rescls;

private:
	jive::output * origin_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

}	//jive namespace

struct jive_variable *
jive_input_get_constraint(const jive::input * self);

void
jive_input_unassign_ssavar(jive::input * self);

struct jive_ssavar *
jive_input_auto_assign_variable(jive::input * self);

struct jive_ssavar *
jive_input_auto_merge_variable(jive::input * self);

namespace jive {

/**	@}	*/

/**
        \defgroup jive::output Outputs
        Outputs
        @{
*/

class output {
public:
	virtual ~output() noexcept;

protected:
	output(struct jive_node * node, size_t index);

public:
	virtual const jive::base::type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;

	inline jive_node * node() const noexcept { return node_; }

	inline bool no_user() const noexcept { return users.first == nullptr; }

	inline bool single_user() const noexcept
		{ return (users.first != nullptr) && (users.first == users.last); }

	size_t index;
	
	struct {
		jive::input * first;
		jive::input * last;
	} users;
	
	jive::gate * gate;
	struct {
		jive::output * prev;
		jive::output * next;
	} gate_outputs_list;
	
	struct jive_ssavar * ssavar;
	
	struct {
		struct jive_ssavar * first;
		struct jive_ssavar * last;
	} originating_ssavars;
	
	const struct jive_resource_class * required_rescls;
private:
	jive_node * node_;
};

}	//jive namespace

struct jive_variable *
jive_output_get_constraint(const jive::output * self);

void
jive_output_replace(jive::output * self, jive::output * other);

struct jive_ssavar *
jive_output_auto_assign_variable(jive::output * self);

struct jive_ssavar *
jive_output_auto_merge_variable(jive::output * self);

namespace jive {

/**	@}	*/

/**
        \defgroup jive::gate Gates
        Gates
        @{
*/

class gate {
public:
	virtual ~gate() noexcept;

protected:
	gate(struct jive_graph * graph, const char name[]);

public:
	virtual const jive::base::type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;

	jive::input * create_input(jive_node * node, size_t index, jive::output * origin);

	jive::output * create_output(jive_node * node, size_t index);
	
	struct jive_graph * graph;
	struct {
		jive::gate * prev;
		jive::gate * next;
	} graph_gate_list;

	std::string name;

	struct {
		jive::input * first;
		jive::input * last;
	} inputs;
	
	struct {
		jive::output * first;
		jive::output * last;
	} outputs;
	
	bool may_spill;
	jive_gate_interference_hash interference;
	
	struct jive_variable * variable;
	struct {
		jive::gate * prev;
		jive::gate * next;
	} variable_gate_list;
	
	const struct jive_resource_class * required_rescls;
};

}	//jive namespace

struct jive_variable *
jive_gate_get_constraint(jive::gate * self);

size_t
jive_gate_interferes_with(const jive::gate * self, const jive::gate * other);

void
jive_gate_merge(jive::gate * self, jive::gate * other);

void
jive_gate_split(jive::gate * self);

void
jive_gate_auto_merge_variable(jive::gate * self);

/**	@}	*/

/**	@}	*/

void
jive_raise_type_error(const jive::base::type * self, const jive::base::type * other);

inline jive_node *
jive::input::producer() const noexcept
{
	return origin_->node();
}

#endif
