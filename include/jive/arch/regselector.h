#ifndef JIVE_ARCH_REGSELECTOR_H
#define JIVE_ARCH_REGSELECTOR_H

#include <stdint.h>

#include <jive/types/bitstring/arithmetic.h>
#include <jive/vsdg/negotiator.h>

struct jive_graph;
struct jive_input;
struct jive_output;
struct jive_resource_class;
struct jive_type;

typedef int jive_regselect_index;
typedef uint32_t jive_regselect_mask;

typedef struct jive_regselector jive_regselector;
typedef struct jive_reg_classifier jive_reg_classifier;

struct jive_reg_classifier {
	jive_regselect_mask any;
	jive_regselect_mask (*classify_type)(const struct jive_type * type, const struct jive_resource_class * rescls);
	jive_regselect_mask (*classify_fixed_arithmetic)(jive_bitop_code op, size_t nbits);
	jive_regselect_mask (*classify_fixed_compare)(jive_bitop_code op, size_t nbits);
	jive_regselect_mask (*classify_address)(void);
	
	const jive_register_class * const * classes;
};

struct jive_regselector {
	jive_negotiator base;
	const jive_reg_classifier * classifier;
};

void
jive_regselector_init(jive_regselector * self, struct jive_graph * graph, const jive_reg_classifier * classifier);

void
jive_regselector_process(jive_regselector * self);

const jive_register_class *
jive_regselector_map_output(const jive_regselector * self, struct jive_output * output);

const jive_register_class *
jive_regselector_map_input(const jive_regselector * self, struct jive_input * input);

void
jive_regselector_fini(jive_regselector * self);

#endif
