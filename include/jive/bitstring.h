#ifndef JIVE_BITSTRING_H
#define JIVE_BITSTRING_H

#include <jive/graph.h>
#include <jive/types.h>

/* FIXME */
#include <jive/nodeclass.h>

/* bitstring base classes */

typedef struct _jive_bitstring_value_range jive_bitstring_value_range;
typedef struct _jive_value_bits jive_value_bits;
typedef struct _jive_operand_bits jive_operand_bits;

extern const jive_value_class JIVE_VALUE_BITS;
extern const jive_operand_class JIVE_OPERAND_BITS;

size_t
jive_bitstring_ninputs(const jive_node * node);

jive_value *
jive_bitstring_input(const jive_node * node, size_t index);

extern const jive_node_class JIVE_BITSTRING_NODE;

/* bitsymbolicconstant */

extern const jive_node_class JIVE_BITSYMBOLICCONSTANT;

jive_node *
jive_bitsymbolicconstant_rawcreate(jive_graph * graph, const char * name, size_t nbits);

jive_value *
jive_bitsymbolicconstant(jive_graph * graph, const char * name, size_t nbits);

const char *
jive_bitsymbolicconstant_name(const jive_node * node);

bool
jive_match_bitsymbolicconstant_node(const jive_node * node, const char * name, size_t nbits);

void
jive_bitsymbolicconstant_set_value_range(jive_node * node, const jive_bitstring_value_range * value_range);

void
jive_bitsymbolicconstant_set_value_range_numeric(jive_node * node, long low, long high);

/* bitconstant */

extern const jive_node_class JIVE_BITCONSTANT;

typedef struct _jive_bitconstant_nodedata {
	const char * bits;
	size_t nbits;
} jive_bitconstant_nodedata;

jive_node *
jive_bitconstant_rawcreate(jive_graph * graph, size_t nbits, const char * bits);

jive_value *
jive_bitconstant(jive_graph * graph, size_t nbits, const char * bits);

jive_value *
jive_bitconstant_with_value(jive_graph * graph, size_t nbits, long value);

const jive_bitconstant_nodedata *
jive_bitconstant_info(const jive_node * node);

bool
jive_match_bitconstant_node(const jive_node * node, size_t nbits, const char * bits);

/**
	\brief Extend bitstring and slice result
	\param value Source bitstring
	\param low Low bit
	\param high High bit
	\param sign_extend Perform sign extension
	
	Logically extends the given bitstring
	infinitely to both sides, using 0 for
	less significant bits, and either 0 or the
	current sign bit for the more significant
	bits. This infinite sequence of bits is
	then sliced within the specified boundaries.
*/

jive_value *
jive_extend_slice(jive_value * value, int low, int high, bool sign_extend);

/* bitslice */

extern const jive_node_class JIVE_BITSLICE;

typedef struct _jive_bitslice_nodedata {
	unsigned short low, high;
} jive_bitslice_nodedata;

jive_node *
jive_bitslice_rawcreate(jive_value * input, size_t low, size_t high);

jive_value *
jive_bitslice(jive_value * input, size_t low, size_t high);

const jive_bitslice_nodedata *
jive_bitslice_info(const jive_node * node);

bool
jive_bitslice_normalized(const jive_node * _node);

bool
jive_match_bitslice_node(const jive_node * node, size_t low, size_t high);

/* bitconcat */

extern const jive_node_class JIVE_BITCONCAT;

jive_node *
jive_bitconcat_rawcreate(size_t ninputs, jive_value * const inputs[]);

jive_value *
jive_bitconcat(size_t ninputs, jive_value * const inputs[]);

/**
	\brief Returns false if node can be further simplified
*/
bool
jive_bitconcat_normalized(const jive_node * node);

bool
jive_match_bitconcat_node(const jive_node * node, size_t ninputs, jive_value * const inputs[]);

/* negation */

extern const jive_node_class JIVE_INTNEG;

jive_node *
jive_intneg_rawcreate(jive_value * input);

/* FIXME: strictly speaking, negation is not a "normalized" representation */
jive_value *
jive_intneg(jive_value * input);

/* sum */

extern const jive_node_class JIVE_INTSUM;

jive_node *
jive_intsum_rawcreate(size_t ninputs, jive_value * const inputs[]);

jive_value *
jive_intsum(size_t ninputs, jive_value * const inputs[]);

/* product */

extern const jive_node_class JIVE_INTPRODUCT;

jive_node *
jive_intproduct_rawcreate(size_t ninputs, jive_value * const inputs[]);

jive_value *
jive_intproduct(size_t ninputs, jive_value * const inputs[]);

/* FIXME: those product types below should be removed */

/* low product */

extern const jive_node_class JIVE_INTLOWPRODUCT;

jive_node *
jive_intlowproduct_rawcreate(size_t ninputs, jive_value * const inputs[]);

jive_value *
jive_intlowproduct(size_t ninputs, jive_value * const inputs[]);

/* signed high product */

extern const jive_node_class JIVE_INTSIGNEDHIGHPRODUCT;

jive_node *
jive_intsignedhiproduct_rawcreate(size_t ninputs, jive_value * const inputs[]);

jive_value *
jive_intsignedhiproduct(size_t ninputs, jive_value * const inputs[]);

/* unsigned high product */

extern const jive_node_class JIVE_INTUNSIGNEDHIGHPRODUCT;

jive_node *
jive_intunsignedhiproduct_rawcreate(size_t ninputs, jive_value * const inputs[]);

jive_value *
jive_intunsignedhiproduct(size_t ninputs, jive_value * const inputs[]);

/* auxiliary functions */

const jive_bitstring_value_range *
jive_value_bits_get_value_range(const jive_value * value);

void
jive_bitstring_value_range_numeric(jive_bitstring_value_range * value_range);

void
jive_bitstring_value_range_bits(jive_bitstring_value_range * value_range);

void
jive_operand_bits_init(jive_operand_bits * input, jive_value * value, unsigned int index);

void
jive_value_bits_init(jive_value_bits * value, jive_node * node, unsigned int nbits);

/** \brief Test if inputs of given node match given array of ports */
bool
jive_match_bitstring_node_inputs(const jive_node * node, size_t ninputs, jive_value * const inputs[]);

/* TODO */

extern const jive_node_class
	JIVE_BITAND,
	JIVE_BITOR,
	JIVE_BITXOR;


/* public structures */

struct _jive_bitstring_value_range {
	char * bits;
	long low, high;
	bool uptodate, numeric;
	unsigned short nbits;
};

#define JIVE_VALUE_BITS_FIELDS \
	JIVE_VALUE_COMMON_FIELDS \
	unsigned short nbits; \
	/* used by register allocator */ \
	jive_bitstring_value_range _value_range; \

struct _jive_value_bits {
	JIVE_VALUE_BITS_FIELDS
};

struct _jive_operand_bits {
	JIVE_OPERAND_COMMON_FIELDS(jive_value_bits)
	unsigned int index;
};

static inline size_t
jive_value_nbits(const jive_value * value)
{
	/* FIXME: activate assertion */
	/* DEBUG_ASSERT(value->type == &JIVE_VALUE_BITS); */
	return ((const jive_value_bits *) value)->nbits;
}

#endif
