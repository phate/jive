/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/real/rlconstant.h>
#include <jive/types/real/rltype.h>
#include <jive/util/buffer.h>
#include <jive/util/double.h>
#include <jive/util/float.h>
#include <jive/util/math.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/nullary.h>

static void
jive_rlconstant_node_init_(jive_rlconstant_node * self, jive_region * region, bool sign,
	size_t nnbits, const char * numerator, size_t ndbits, const char * denominator);

static void
jive_rlconstant_node_fini_(jive_node * self);

static void
jive_rlconstant_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static const jive_node_attrs *
jive_rlconstant_node_get_attrs_(const jive_node * self);

static bool
jive_rlconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_rlconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_RLCONSTANT_NODE = {
	.parent = &JIVE_NULLARY_OPERATION,
	.fini = jive_rlconstant_node_fini_, /* override */
	.get_default_normal_form = jive_nullary_operation_get_default_normal_form_, /* inherit */
	.get_label = jive_rlconstant_node_get_label_, /* override */
	.get_attrs = jive_rlconstant_node_get_attrs_, /* override */
	.match_attrs = jive_rlconstant_node_match_attrs_, /* override */
	.create = jive_rlconstant_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_rlconstant_node_init_(jive_rlconstant_node * self, jive_region * region, bool sign,
	size_t nnbits, const char * numerator, size_t ndbits, const char * denominator)
{
	JIVE_DECLARE_REAL_TYPE(rltype);
	jive_node_init_(&self->base, region,
		0, NULL, NULL,
		1, &rltype);

	self->attrs.sign = sign;

	size_t n;
	self->attrs.nnbits = nnbits;
	self->attrs.numerator = jive_context_malloc(region->graph->context, nnbits);
	for (n = 0; n < nnbits; n++)
		self->attrs.numerator[n] = numerator[n];

	self->attrs.ndbits = ndbits;
	self->attrs.denominator = jive_context_malloc(region->graph->context, ndbits);
	for (n = 0; n < ndbits; n++)
		self->attrs.denominator[n] = denominator[n];
}

static void
jive_rlconstant_node_fini_(jive_node * self_)
{
	jive_rlconstant_node * self = (jive_rlconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.numerator);
	jive_context_free(self->base.graph->context, self->attrs.denominator);
	jive_node_fini_(&self->base);
}

static void
jive_rlconstant_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_rlconstant_node * self = (const jive_rlconstant_node *) self_;

	if (self->attrs.sign)
		jive_buffer_putstr(buffer, "-(");
	else
		jive_buffer_putstr(buffer, "+(");

	size_t n;
	char tmp[jive_max_unsigned(self->attrs.nnbits, self->attrs.ndbits)+1];
	for (n = 0; n < self->attrs.nnbits; n++)
		tmp[n] = self->attrs.numerator[self->attrs.nnbits-n-1];
	tmp[n] = 0;
	jive_buffer_putstr(buffer, tmp);

	jive_buffer_putstr(buffer, "/");
	for (n = 0; n < self->attrs.ndbits; n++)
		tmp[n] = self->attrs.denominator[self->attrs.ndbits-n-1];
	tmp[n] = 0;
	jive_buffer_putstr(buffer, tmp);
	jive_buffer_putstr(buffer, ")");
}

static const jive_node_attrs *
jive_rlconstant_node_get_attrs_(const jive_node * self_)
{
	const jive_rlconstant_node * self = (const jive_rlconstant_node *) self_;
	return &self->attrs.base;
}

static bool
jive_rlconstant_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_rlconstant_node_attrs * first = &((const jive_rlconstant_node *) self)->attrs;
	const jive_rlconstant_node_attrs * second = (const jive_rlconstant_node_attrs *) attrs;

	if (first->sign != second->sign)
		return false;

	if (first->nnbits != second->nnbits || first->ndbits != second->ndbits)
		return false;

	size_t n;
	for (n = 0; n < first->nnbits; n++) {
		if (first->numerator[n] != second->numerator[n])
			return false;
	}

	for (n = 0; n < first->ndbits; n++) {
		if (first->denominator[n] != second->denominator[n])
			return false;
	}

	return true;
}

static jive_node *
jive_rlconstant_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_rlconstant_node_attrs * attrs = (const jive_rlconstant_node_attrs *) attrs_;

	jive_rlconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_RLCONSTANT_NODE;
	jive_rlconstant_node_init_(node, region, attrs->sign, attrs->nnbits, attrs->numerator,
		attrs->ndbits, attrs->denominator);

	return &node->base;
}

struct jive_output *
jive_rlconstant(struct jive_graph * graph, bool sign, size_t nnbits, const char * numerator,
	size_t ndbits, const char * denominator)
{
	size_t nbits = jive_max_unsigned(nnbits, ndbits);

	char n[nbits], d[nbits];
	jive_bitstring_extend_unsigned(n, nbits, numerator, nnbits);
	jive_bitstring_extend_unsigned(d, nbits, denominator, ndbits);

	if (jive_bitstring_is_known(n, nbits) && jive_bitstring_is_known(d, nbits) &&
		!jive_bitstring_is_zero(d, nbits)) {
		char gcd[nbits], r[nbits];
		jive_bitstring_gcd(gcd, n, d, nbits);

		if (!jive_bitstring_is_one(gcd, nbits)) {
			jive_bitstring_division_unsigned(n, r, n, gcd, nbits);
			jive_bitstring_division_unsigned(d, r, d, gcd, nbits);
		}
	}

	jive_rlconstant_node_attrs attrs;
	attrs.sign = jive_bitstring_is_zero(n, nbits) ? false : sign;
	attrs.nnbits = jive_max_unsigned(nbits-jive_bitstring_nlz(n, nbits), 1);
	attrs.numerator = (char *) n;
	attrs.ndbits = jive_max_unsigned(nbits-jive_bitstring_nlz(d, nbits), 1);
	attrs.denominator = (char *) d;

	return jive_nullary_operation_create_normalized(&JIVE_RLCONSTANT_NODE, graph, &attrs.base);
}

struct jive_output *
jive_rlconstant_float(struct jive_graph * graph, float f)
{
	size_t nbits = 150;
	char n[nbits], d[nbits];
	if (jive_float_is_value(f)) {
		uint32_t m = jive_float_raw_mantissa(f);

		uint32_t i;
		uint32_t s = 0;
		for (i = 1; i <= 23; i++)
			s += ((m >> (23-i)) & 1) * (1 << i);

		jive_bitstring_init_unsigned(d, nbits, s);

		if (jive_float_is_normalized(f)) {
			jive_bitstring_init_unsigned(n, nbits, 1);
			jive_bitstring_sum(n, n, d, nbits);

			int8_t e = jive_float_decoded_exponent(f);
			if (e >= 0)
				jive_bitstring_shiftleft(n, n, nbits, e);
			else
				jive_bitstring_shiftleft(d, d, nbits, -e);

			/*
					If the significands are zero, d will also be zero. Thus, we need to set
					it to one to get a valid rational number.
			*/
			if (s == 0)
				jive_bitstring_init_unsigned(d, nbits, 1);
		} else {
			if (s == 0) { /* -0.0 and 0.0 */
				jive_bitstring_init_unsigned(n, nbits, 0);
				jive_bitstring_init_unsigned(d, nbits, 1);
			} else { /* denormalized numbers */
				char tmp[nbits];
				jive_bitstring_init_unsigned(n, nbits, 1);
				jive_bitstring_init_unsigned(tmp, nbits, 1);
				jive_bitstring_shiftleft(tmp, tmp, nbits, 126);
				jive_bitstring_sum(d, d, tmp, nbits);
			}
		}
	} else { /* +/-infinity and NaN */
		jive_bitstring_init_unsigned(n, nbits, 0);
		jive_bitstring_init_unsigned(d, nbits, 0);
	}

	bool sign = jive_float_is_signed(f) && !jive_float_is_signed_zero(f);
	return jive_rlconstant(graph, sign, nbits, n, nbits, d);
}

struct jive_output *
jive_rlconstant_double(struct jive_graph * graph, double f)
{
	size_t nbits = 1075;
	char n[nbits], d[nbits];
	if (jive_double_is_value(f)) {
		uint64_t m = jive_double_raw_mantissa(f);

		uint32_t i;
		uint64_t s = 0;
		for (i = 1; i <= 52; i++)
			s += ((m >> (52-i)) & 1) * (1 << i);

		jive_bitstring_init_unsigned(d, nbits, s);

		if (jive_double_is_normalized(f)) {
			jive_bitstring_init_unsigned(n, nbits, 1);
			jive_bitstring_sum(n, n, d, nbits);

			int16_t e = jive_double_decoded_exponent(f);
			if (e >= 0)
				jive_bitstring_shiftleft(n, n, nbits, e);
			else
				jive_bitstring_shiftleft(d, d, nbits, -e);

			/*
				If the significands are zero, d will also be zero. Thus, we need to set
				it to one to get a valid rational number.
			*/
			if (s == 0)
				jive_bitstring_init_unsigned(d, nbits, 1);
		} else {
			if (s == 0) { /* -0.0 and 0.0 */
				jive_bitstring_init_unsigned(n, nbits, 0);
				jive_bitstring_init_unsigned(d, nbits, 1);
			} else { /* denormalized numbers */
				char tmp[nbits];
				jive_bitstring_init_unsigned(n, nbits, 1);
				jive_bitstring_init_unsigned(tmp, nbits, 1);
				jive_bitstring_shiftleft(tmp, tmp, nbits, 1022);
				jive_bitstring_sum(d, d, tmp, nbits);
			}
		}
	} else { /* +/-infinity and NaN */
		jive_bitstring_init_unsigned(n, nbits, 0);
		jive_bitstring_init_unsigned(d, nbits, 0);
	}

	bool sign = jive_double_is_signed(f) && !jive_double_is_signed_zero(f);
	return jive_rlconstant(graph, sign, nbits, n, nbits, d);
}
