#ifndef JIVE_BITSTRING_BITOPS_H
#define JIVE_BITSTRING_BITOPS_H

#include <string.h>

/**
	\brief Compute logic or
	\param a
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param b
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\return Logic "or" of bits
		
	Performs logical "or"
*/
static inline char
jive_logic_or(char a, char b)
{
	switch(a) {
		case '0':
			return b;
		case '1':
			return '1';
		case 'X':
			if (b == '1') return '1';
			return 'X';
		case 'D':
			if (b == '1') return '1';
			if (b == 'X') return 'X';
			return 'D';
		default:
			return 'X';
	}
}

/**
	\brief Compute logic xor
	\param a
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param b
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\return Logic "xor" of bits
		
	Performs logical "xor"
*/
static inline char
jive_logic_xor(char a, char b)
{
	switch(a) {
		case '0':
			return b;
		case '1':
			if (b == '1') return '0';
			if (b == '0') return '1';
			return b;
		case 'X':
			return 'X';
		case 'D':
			if (b == 'X') return 'X';
			return a;
		default:
			return 'X';
	}
}

/**
	\brief Compute logic not
	\param a
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\return Logic "not" of bit
		
	Performs logical "not"
*/
static inline char
jive_logic_not(char a)
{
	return jive_logic_xor('1', a);
}

/**
	\brief Compute logic and
	\param a
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param b
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\return Logic "and" of bits
		
	Performs logical "and"
*/
static inline char
jive_logic_and(char a, char b)
{
	switch(a) {
		case '0':
			return '0';
		case '1':
			return b;
		case 'X':
			if (b == '0') return '0';
			return 'X';
		case 'D':
			if (b == '0') return '0';
			if (b == 'X') return 'X';
			return 'D';
		default:
			return 'X';
	}
}


static inline char
jive_bitnotequal_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	char val = '0';
	size_t n;
	for(n = 0; n < nbits; n++) {
		val = jive_logic_or(val, jive_logic_xor(c1[n], c2[n]));
	}
	return val;
}


static inline char
jive_bitequal_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	return jive_logic_not(jive_bitnotequal_compare_constants_(c1, c2, nbits));
}

static inline char
jive_bituless_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	size_t n;
	char val = jive_logic_and(jive_logic_not(c1[0]), c2[0]);
	for(n = 1; n < nbits; n++){
		val = jive_logic_and(
			jive_logic_or(jive_logic_not(c1[n]), c2[n]),
			jive_logic_or(jive_logic_and(jive_logic_not(c1[n]), c2[n]), val));
	}
	return val;
}

static inline char
jive_bitsless_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	char t1[nbits];
	char t2[nbits];
	memcpy(t1, c1, nbits);
	memcpy(t2, c2, nbits);	
	
	t1[nbits-1] = jive_logic_not(t1[nbits-1]);
	t2[nbits-1] = jive_logic_not(t2[nbits-1]);
	
	return jive_bituless_compare_constants_(t1, t2, nbits);
}

static inline char
jive_bitslesseq_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	//FIXME: summarize to one formula to get a more precise result
	char l = jive_bitsless_compare_constants_(c1, c2, nbits);
	char e = jive_bitequal_compare_constants_(c1, c2, nbits);
	return jive_logic_or(l, e);
}

static inline char
jive_bitulesseq_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	//FIXME: summarize to one formula to get a more precise result
	char l = jive_bituless_compare_constants_(c1, c2, nbits);
	char e = jive_bitequal_compare_constants_(c1, c2, nbits);
	return jive_logic_or(l, e);
}

static inline char
jive_bitsgreater_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	return jive_logic_not(jive_bitslesseq_compare_constants_(c1, c2, nbits));
}

static inline char
jive_bitugreater_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	return jive_logic_not(jive_bitulesseq_compare_constants_(c1, c2, nbits));
}

static inline char
jive_bitsgreatereq_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	return jive_logic_not(jive_bitsless_compare_constants_(c1, c2, nbits));
}

static inline char
jive_bitugreatereq_compare_constants_(const char * c1, const char * c2, size_t nbits)
{
	return jive_logic_not(jive_bituless_compare_constants_(c1, c2, nbits));
}

/**
	\brief Compute logic carry
	\param a
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param b
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param c
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\return Carry that occurs when adding the three input bits
		
	Compute carry that results from the sum of the three parameters
*/
static inline char
jive_logic_carry(char a, char b, char c)
{
	return jive_logic_or(jive_logic_or(jive_logic_and(a,b),jive_logic_and(a,c)),jive_logic_and(b,c));
}

/**
	\brief Compute single-bit sum
	\param a
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param b
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\param c
		bit; must be '0', '1', 'X' (undefined) or 'D' (defined,
		but unknown)
	\return Sum of three input bits
		
	Compute sum of the three parameters (not accounting for overflow)
*/
static inline char
jive_logic_add(char a, char b, char c)
{
	return jive_logic_xor(jive_logic_xor(a,b),c);
}

/**
	\brief Compute multi-bit sum
	\param sum
		bitstring into which sum should be computed
	\param op1
		bitstring, first operand
	\param op2
		bitstring, second operand
	\param nbits
		number of bits per operand
*/
static inline void
jive_multibit_sum(
	char sum[], const char op1[], const char op2[],
	size_t nbits)
{
	unsigned int n;
	char carry='0';
	for(n=0; n<nbits; n++) {
		sum[n]=jive_logic_add(op1[n], op2[n], carry);
		carry=jive_logic_carry(op1[n], op2[n], carry);
	}
}

static inline void
jive_multibit_and(char dst[], const char op1[], const char op2[], size_t nbits)
{
	size_t n;
	for(n = 0; n < nbits; n++){
		dst[n] = jive_logic_and(op1[n], op2[n]);
	}	
}

static inline void
jive_multibit_or(char dst[], const char op1[], const char op2[], size_t nbits)
{
	size_t n;
	for(n = 0; n < nbits; n++){
		dst[n] = jive_logic_or(op1[n], op2[n]);
	}
}

static inline void
jive_multibit_xor(char dst[], const char op1[], const char op2[], size_t nbits)
{
	size_t n;
	for(n = 0; n < nbits; n++){
		dst[n] = jive_logic_xor(op1[n], op2[n]);
	}
}

static inline void
jive_multibit_not(char dst[], const char op[], size_t nbits)
{
	char tmp[nbits];
	memset(tmp, '1', nbits);
	jive_multibit_xor(dst, op, tmp, nbits);
}

static inline void
jive_multibit_negate(
	char dst[], const char src[], size_t nbits)
{
	char carry = '1';
	size_t n;
	for(n = 0; n<nbits; n++) {
		char tmp = jive_logic_xor(src[n], '1');
		dst[n] = jive_logic_add(tmp, '0', carry);
		carry = jive_logic_carry(tmp, '0', carry);
	}
}

static inline void
jive_multibit_difference(char dst[],
	const char dividend[], const char divisor[], size_t nbits)
{
	char tmp[nbits];
	jive_multibit_negate(tmp, divisor, nbits);
	jive_multibit_sum(dst, dividend, tmp, nbits);
}

static inline void
jive_bitstring_init_unsigned(char dst[], size_t nbits, uint64_t value)
{
	size_t i;
	for(i = 0; i < nbits; i++){
		dst[i] = '0' + (value & 1);
		value = value >> 1;
	}	
}

static inline void
jive_bitstring_init_signed(char dst[], size_t nbits, int64_t value)
{
	jive_bitstring_init_unsigned(dst, nbits, (uint64_t)value);
}

static inline void
jive_multibit_shiftright(char dst[],
	const char operand[], size_t nbits, size_t shift)
{
	memset(dst, '0', nbits);
	if(shift >= nbits) return;

	memcpy(dst, operand+shift, nbits-shift);
}

static inline void
jive_multibit_shiftleft(char dst[],
	const char operand[], size_t nbits, size_t shift)
{
	memset(dst, '0', nbits);
	if(shift >= nbits) return;

	memcpy(dst+shift, operand, nbits-shift);
}

static inline void
jive_multibit_arithmetic_shiftright(char dst[],
	const char operand[], size_t nbits, size_t shift)
{
	memset(dst, operand[nbits-1], nbits);
	if(shift >= nbits) return;

	memcpy(dst, operand+shift, nbits-shift);
}

static inline void
jive_multibit_multiply(
	char product[], size_t product_nbits,
	const char factor1[], size_t factor1_nbits,
	const char factor2[], size_t factor2_nbits)
{
	unsigned int n1, p1=0;
	memset(product, '0', product_nbits);
	for(n1=0; n1<product_nbits; n1++) {
		char b1 = factor1[p1];
		if (p1<factor1_nbits-1) p1++;
		unsigned int n2, p2 = 0, t = n1;
		char carry = '0';
		for(n2=0; t<product_nbits; n2++,t++) {
			char b2 = factor2[p2];
			if (p2<factor2_nbits-1) p2++;
			
			char s = jive_logic_and(b1, b2);
			char new_carry = jive_logic_carry(s, product[t], carry);
			product[t] = jive_logic_add(s, product[t], carry);
			carry = new_carry;
		}
	}
}

static inline void
jive_multibit_multiply_shiftright(
	char product[], const char factor1[], const char factor2[],
	size_t nbits, size_t shift)
{
	char tmp[nbits+shift];
	jive_multibit_multiply(product, shift+nbits, factor1, nbits, factor2, nbits);
	memcpy(product, tmp+shift, nbits);
}

#endif
