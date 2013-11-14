/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdint.h>

#include <jive/util/bitstring.h>
#include <jive/util/math.h>

char s[] =
	{'0', '1', 'D', 'X'};

char not[] =
	{'1', '0', 'D', 'X'};

char or[4][4] = {
	{'0', '1', 'D', 'X'},
	{'1', '1', '1', '1'},
	{'D', '1', 'D', 'X'},
	{'X', '1', 'X', 'X'}};

char xor [4][4] = {
	{'0', '1', 'D', 'X'},
	{'1', '0', 'D', 'X'},
	{'D', 'D', 'D', 'X'},
	{'X', 'X', 'X', 'X'}};

char and[4][4] = {
	{'0', '0', '0', '0'},
	{'0', '1', 'D', 'X'},
	{'0', 'D', 'D', 'X'},
	{'0', 'X', 'X', 'X'}};

const char * bs[] = {
	"00000000",
	"11111111",
	"10000000",
	"01111111",
	"00001111",
	"XXXX0011",
	"XD001100",
	"XXXXDDDD",
	"10XDDX01",
	"0DDDDDD1"};

const char * bitstring_not[10] = {
	"11111111",
	"00000000",
	"01111111",
	"10000000",
	"11110000",
	"XXXX1100",
	"XD110011",
	"XXXXDDDD",
	"01XDDX10",
	"1DDDDDD0"};

const char * bitstring_xor[10][10] = {
	{"00000000", "11111111", "10000000", "01111111", "00001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"10XDDX01", "0DDDDDD1"},
	{"11111111", "00000000", "01111111", "10000000", "11110000", "XXXX1100", "XD110011", "XXXXDDDD",
		"01XDDX10", "1DDDDDD0"},
	{"10000000", "01111111", "00000000", "11111111", "10001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"00XDDX01", "1DDDDDD1"},
	{"01111111", "10000000", "11111111", "00000000", "01110000", "XXXX1100", "XD110011", "XXXXDDDD",
		"11XDDX10", "0DDDDDD0"},
	{"00001111", "11110000", "10001111", "01110000", "00000000", "XXXX1100", "XD000011", "XXXXDDDD",
		"10XDDX10", "0DDDDDD0"},
	{"XXXX0011", "XXXX1100", "XXXX0011", "XXXX1100", "XXXX1100", "XXXX0000", "XXXX1111", "XXXXDDDD",
		"XXXXDX10", "XXXXDDD0"},
	{"XD001100", "XD110011", "XD001100", "XD110011", "XD000011", "XXXX1111", "XD000000", "XXXXDDDD",
		"XDXDDX01", "XDDDDDD1"},
	{"XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD",
		"XXXXDXDD", "XXXXDDDD"},
	{"10XDDX01", "01XDDX10", "00XDDX01", "11XDDX10", "10XDDX10", "XXXXDX10", "XDXDDX01", "XXXXDXDD",
		"00XDDX00", "1DXDDXD0"},
	{"0DDDDDD1", "1DDDDDD0", "1DDDDDD1", "0DDDDDD0", "0DDDDDD0", "XXXXDDD0", "XDDDDDD1", "XXXXDDDD",
		"1DXDDXD0", "0DDDDDD0"}};

const char * bitstring_or[10][10] = {
	{"00000000", "11111111", "10000000", "01111111", "00001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"10XDDX01", "0DDDDDD1"},
	{"11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111",
		"11111111", "11111111"},
	{"10000000", "11111111", "10000000", "11111111", "10001111", "1XXX0011", "1D001100", "1XXXDDDD",
		"10XDDX01", "1DDDDDD1"},
	{"01111111", "11111111", "11111111", "01111111", "01111111", "X1111111", "X1111111", "X1111111",
		"11111111", "01111111"},
	{"00001111", "11111111", "10001111", "01111111", "00001111", "XXXX1111", "XD001111", "XXXX1111",
		"10XD1111", "0DDD1111"},
	{"XXXX0011", "11111111", "1XXX0011", "X1111111", "XXXX1111", "XXXX0011", "XXXX1111", "XXXXDD11",
		"1XXXDX11", "XXXXDD11"},
	{"XD001100", "11111111", "1D001100", "X1111111", "XD001111", "XXXX1111", "XD001100", "XXXX11DD",
		"1DXD1101", "XDDD11D1"},
	{"XXXXDDDD", "11111111", "1XXXDDDD", "X1111111", "XXXX1111", "XXXXDD11", "XXXX11DD", "XXXXDDDD",
		"1XXXDXD1", "XXXXDDD1"},
	{"10XDDX01", "11111111", "10XDDX01", "11111111", "10XD1111", "1XXXDX11", "1DXD1101", "1XXXDXD1",
		"10XDDX01", "1DXDDXD1"},
	{"0DDDDDD1", "11111111", "1DDDDDD1", "01111111", "0DDD1111", "XXXXDD11", "XDDD11D1", "XXXXDDD1",
		"1DXDDXD1", "0DDDDDD1"}};

const char * bitstring_and[10][10] = {
	{"00000000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000000",
		"00000000", "00000000"},
	{"00000000", "11111111", "10000000", "01111111", "00001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"10XDDX01", "0DDDDDD1"},
	{"00000000", "10000000", "10000000", "00000000", "00000000", "X0000000", "X0000000", "X0000000",
		"10000000", "00000000"},
	{"00000000", "01111111", "00000000", "01111111", "00001111", "0XXX0011", "0D001100", "0XXXDDDD",
		"00XDDX01", "0DDDDDD1"},
	{"00000000", "00001111", "00000000", "00001111", "00001111", "00000011", "00001100", "0000DDDD",
		"0000DX01", "0000DDD1"},
	{"00000000", "XXXX0011", "X0000000", "0XXX0011", "00000011", "XXXX0011", "XX000000", "XXXX00DD",
		"X0XX0001", "0XXX00D1"},
	{"00000000", "XD001100", "X0000000", "0D001100", "00001100", "XX000000", "XD001100", "XX00DD00",
		"X000DX00", "0D00DD00"},
	{"00000000", "XXXXDDDD", "X0000000", "0XXXDDDD", "0000DDDD", "XXXX00DD", "XX00DD00", "XXXXDDDD",
		"X0XXDX0D", "0XXXDDDD"},
	{"00000000", "10XDDX01", "10000000", "00XDDX01", "0000DX01", "X0XX0001", "X000DX00", "X0XXDX0D",
		"10XDDX01", "00XDDX01"},
	{"00000000", "0DDDDDD1", "00000000", "0DDDDDD1", "0000DDD1", "0XXX00D1", "0D00DD00", "0XXXDDDD",
		"00XDDX01", "0DDDDDD1"}};

char equal[10][10] = {
	{'1', '0', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '1', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '0', '1', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '0', '0', '1', '0', '0', '0', 'X', '0', 'D'},
	{'0', '0', '0', '0', '1', '0', '0', 'X', '0', 'D'},
	{'0', '0', '0', '0', '0', 'X', '0', 'X', '0', 'X'},
	{'0', '0', '0', '0', '0', '0', 'X', 'X', '0', '0'},
	{'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'0', '0', '0', '0', '0', '0', '0', 'X', 'X', '0'},
	{'0', '0', '0', 'D', 'D', 'X', '0', 'X', '0', 'D'}};

char notequal[10][10] = {
	{'0', '1', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '0', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '1', '0', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '1', '1', '0', '1', '1', '1', 'X', '1', 'D'},
	{'1', '1', '1', '1', '0', '1', '1', 'X', '1', 'D'},
	{'1', '1', '1', '1', '1', 'X', '1', 'X', '1', 'X'},
	{'1', '1', '1', '1', '1', '1', 'X', 'X', '1', '1'},
	{'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'1', '1', '1', '1', '1', '1', '1', 'X', 'X', '1'},
	{'1', '1', '1', 'D', 'D', 'X', '1', 'X', '1', 'D'}};

char sgreatereq[10][10] = {
	{'1', '1', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '1', '0', '1', '1', '1', '0', 'D', '1', '1'},
	{'1', '1', '1', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '0', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '0', '0', '0', '1', '1', '0', 'X', '1', 'D'},
	{'0', '0', '0', '0', '0', 'X', '0', 'X', '1', 'X'},
	{'1', '1', '1', '1', '1', '1', 'X', 'X', '1', '1'},
	{'D', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'0', '0', '0', '0', '0', '0', '0', 'X', 'X', 'X'},
	{'0', '0', '0', 'D', 'D', 'X', '0', 'X', 'X', 'D'}};

char sgreater[10][10] = {
	{'0', '1', '0', '1', '1', '1', '0', 'D', '1', '1'},
	{'0', '0', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'1', '1', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '0', '0', '0', '1', '1', '0', 'X', '1', 'D'},
	{'0', '0', '0', '0', '0', '1', '0', 'D', '1', 'D'},
	{'0', '0', '0', '0', '0', 'X', '0', 'X', '1', 'X'},
	{'1', '1', '1', '1', '1', '1', 'X', 'X', '1', '1'},
	{'X', 'D', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'0', '0', '0', '0', '0', '0', '0', 'X', 'X', 'X'},
	{'0', '0', '0', '0', 'D', 'X', '0', 'X', 'X', 'D'}};

char slesseq[10][10] = {
	{'1', '0', '1', '0', '0', '0', '1', 'D', '0', '0'},
	{'1', '1', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'0', '0', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '1', '1', '1', '0', '0', '1', 'X', '0', 'D'},
	{'1', '1', '1', '1', '1', '0', '1', 'D', '0', 'D'},
	{'1', '1', '1', '1', '1', 'X', '1', 'X', '0', 'X'},
	{'0', '0', '0', '0', '0', '0', 'X', 'X', '0', '0'},
	{'X', 'D', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'1', '1', '1', '1', '1', '1', '1', 'X', 'X', 'X'},
	{'1', '1', '1', '1', 'D', 'X', '1', 'X', 'X', 'D'}};

char sless[10][10] = {
	{'0', '0', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '0', '1', '0', '0', '0', '1', 'D', '0', '0'},
	{'0', '0', '0', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '1', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '1', '1', '1', '0', '0', '1', 'X', '0', 'D'},
	{'1', '1', '1', '1', '1', 'X', '1', 'X', '0', 'X'},
	{'0', '0', '0', '0', '0', '0', 'X', 'X', '0', '0'},
	{'D', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'1', '1', '1', '1', '1', '1', '1', 'X', 'X', 'X'},
	{'1', '1', '1', 'D', 'D', 'X', '1', 'X', 'X', 'D'}};

char ugreatereq[10][10] = {
	{'1', '0', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'1', '1', '1', '1', '1', '1', '1', '1', '1', '1'},
	{'1', '0', '1', '0', '0', '0', '0', 'X', '0', '0'},
	{'1', '0', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '0', '1', '0', '1', '1', '1', 'X', '1', 'D'},
	{'1', '0', '1', '0', '0', 'X', '1', 'X', '1', 'X'},
	{'1', '0', '1', '0', '0', '0', 'X', 'X', '0', '0'},
	{'1', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'1', '0', '1', '0', '0', '0', '1', 'X', 'X', 'X'},
	{'1', '0', '1', 'D', 'D', 'X', '1', 'X', 'X', 'D'}};

char ugreater[10][10] = {
	{'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
	{'1', '0', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '0', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'1', '0', '1', '0', '1', '1', '1', 'X', '1', 'D'},
	{'1', '0', '1', '0', '0', '1', '1', 'D', '1', 'D'},
	{'1', '0', '1', '0', '0', 'X', '1', 'X', '1', 'X'},
	{'1', '0', '1', '0', '0', '0', 'X', 'X', '0', '0'},
	{'X', '0', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'1', '0', '1', '0', '0', '0', '1', 'X', 'X', 'X'},
	{'1', '0', '1', '0', 'D', 'X', '1', 'X', 'X', 'D'}};

char ulesseq[10][10] = {
	{'1', '1', '1', '1', '1', '1', '1', '1', '1', '1'},
	{'0', '1', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '1', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'0', '1', '0', '1', '0', '0', '0', 'X', '0', 'D'},
	{'0', '1', '0', '1', '1', '0', '0', 'D', '0', 'D'},
	{'0', '1', '0', '1', '1', 'X', '0', 'X', '0', 'X'},
	{'0', '1', '0', '1', '1', '1', 'X', 'X', '1', '1'},
	{'X', '1', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'0', '1', '0', '1', '1', '1', '0', 'X', 'X', 'X'},
	{'0', '1', '0', '1', 'D', 'X', '0', 'X', 'X', 'D'}};


char uless[10][10] = {
	{'0', '1', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
	{'0', '1', '0', '1', '1', '1', '1', 'X', '1', '1'},
	{'0', '1', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '1', '0', '1', '0', '0', '0', 'X', '0', 'D'},
	{'0', '1', '0', '1', '1', 'X', '0', 'X', '0', 'X'},
	{'0', '1', '0', '1', '1', '1', 'X', 'X', '1', '1'},
	{'0', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'0', '1', '0', '1', '1', '1', '0', 'X', 'X', 'X'},
	{'0', '1', '0', 'D', 'D', 'X', '0', 'X', 'X', 'D'}};


static int test_main(void)
{

	int r, c;
	for(r=0; r < 4; r++){
			assert(not[r] == jive_bit_not(s[r]));
		for(c=0; c < 4; c++){
			assert(or[r][c] == jive_bit_or(s[r], s[c]));
			assert(xor[r][c] == jive_bit_xor(s[r], s[c]));
			assert(and[r][c] == jive_bit_and(s[r], s[c]));
		}
	}

	char dst[8];
	for(r=0; r < 10; r++){
			jive_bitstring_not(dst, bs[r], 8);
			assert(!strncmp(dst, bitstring_not[r], 8));

		for(c=0; c < 10; c++){
			jive_bitstring_and(dst, bs[r], bs[c], 8);
			assert(!strncmp(dst, bitstring_and[r][c], 8));

			jive_bitstring_or(dst, bs[r], bs[c], 8);
			assert(!strncmp(dst, bitstring_or[r][c], 8));

			jive_bitstring_xor(dst, bs[r], bs[c], 8);
			assert(!strncmp(dst, bitstring_xor[r][c], 8));

			assert(jive_bitstring_equal(bs[r], bs[c], 8) == equal[r][c]);
			assert(jive_bitstring_notequal(bs[r], bs[c], 8) == notequal[r][c]);

			assert(jive_bitstring_uless(bs[r], bs[c], 8) == uless[r][c]);
			assert(jive_bitstring_sless(bs[r], bs[c], 8) == sless[r][c]);

			assert(jive_bitstring_ulesseq(bs[r], bs[c], 8) == ulesseq[r][c]);
			assert(jive_bitstring_slesseq(bs[r], bs[c], 8) == slesseq[r][c]);

			assert(jive_bitstring_ugreater(bs[r], bs[c], 8) == ugreater[r][c]);
			assert(jive_bitstring_sgreater(bs[r], bs[c], 8) == sgreater[r][c]);

			assert(jive_bitstring_ugreatereq(bs[r], bs[c], 8) == ugreatereq[r][c]);
			assert(jive_bitstring_sgreatereq(bs[r], bs[c], 8) == sgreatereq[r][c]);
		}
	}

	assert(jive_bitstring_to_unsigned("000110", 5) == 24);
	assert(jive_bitstring_to_unsigned("000110", 6) == 24);
	assert(jive_bitstring_to_signed("000110", 5) == -8);
	assert(jive_bitstring_to_signed("000110", 6) == 24);

	char dst128[128];
	jive_bitstring_init_unsigned(dst128, 128, 4);
	assert(jive_bitstring_to_signed(dst128, 64) == 4);
	assert(jive_bitstring_to_signed(dst128+64, 64) == 0);

	jive_bitstring_init_signed(dst128, 128, -4);
	assert(jive_bitstring_to_signed(dst128, 64) == -4);
	assert(jive_bitstring_to_signed(dst128+64, 64) == -1);

	jive_bitstring_init_signed(dst128, 128, 4);
	assert(jive_bitstring_to_signed(dst128, 64) == 4);
	assert(jive_bitstring_to_signed(dst128+64, 64) == 0);

	jive_bitstring_init_unsigned(dst128, 128, 0);
	assert(jive_bitstring_is_zero(dst128, 128));
	assert(!jive_bitstring_is_minus_one(dst128, 128));

	jive_bitstring_init_signed(dst128, 128, -1);
	assert(!jive_bitstring_is_zero(dst128, 128));
	assert(jive_bitstring_is_minus_one(dst128, 128));

	char dst32[32];
	for(r = -4; r < 5; r++){
		char rbits[32];
		jive_bitstring_init_signed(rbits, 32, r);

		char neg[32];
		jive_bitstring_negate(dst32, rbits, 32);
		jive_bitstring_init_signed(neg, 32, -r);
		assert(jive_bitstring_equal(dst32, neg, 32) == '1');

		jive_bitstring_shiftleft(dst32, rbits, 32, 1);
		int64_t value = jive_bitstring_to_signed(dst32, 32);
		assert(value == r << 1);
		jive_bitstring_shiftright(dst32, rbits, 32, 34);
		value = jive_bitstring_to_signed(dst32, 32);
		assert(value == 0);

		jive_bitstring_arithmetic_shiftright(dst32, rbits, 32, 1);
		value = jive_bitstring_to_signed(dst32, 32);
		assert(value == r >> 1);
		jive_bitstring_arithmetic_shiftright(dst32, rbits, 32, 34);
		value = jive_bitstring_to_signed(dst32, 32);
		assert(value == (r < 0 ? -1 : 0));

		if(r >= 0){
			jive_bitstring_shiftright(dst32, rbits, 32, 1);
			int64_t value = jive_bitstring_to_signed(dst32, 32);
			assert(value == r >> 1);
			jive_bitstring_shiftright(dst32, rbits, 32, 34);
			value = jive_bitstring_to_signed(dst32, 32);
			assert(value == 0);
		}

		for(c = -4; c < 5; c++){
			char cbits[32];
			jive_bitstring_init_signed(cbits, 32, c);

			char sum[32];
			jive_bitstring_sum(dst32, rbits, cbits, 32);
			jive_bitstring_init_signed(sum, 32, r+c);
			assert(jive_bitstring_equal(dst32, sum, 32) == '1');

			char diff[32];
			jive_bitstring_difference(dst32, rbits, cbits, 32);
			jive_bitstring_init_signed(diff, 32, r-c);
			assert(jive_bitstring_equal(dst32, diff, 32) == '1');

			if (r >= 0 && c > 0) {
				char quotient[32], remainder[32];
				jive_bitstring_division_unsigned(quotient, remainder, rbits, cbits, 32);

				char div[32], mod[32];
				jive_bitstring_init_signed(div, 32, r/c);
				jive_bitstring_init_signed(mod, 32, r%c);

				assert(jive_bitstring_equal(quotient, div, 32) == '1');
				assert(jive_bitstring_equal(remainder, mod, 32) == '1');

				char gcd[32];
				jive_bitstring_gcd(gcd, rbits, cbits, 32);
				assert(jive_bitstring_equals_unsigned(gcd, 32, jive_gcd(r, c)));

				jive_bitstring_gcd(gcd, cbits, rbits, 32);
				assert(jive_bitstring_equals_unsigned(gcd, 32, jive_gcd(c, r)));
			}

			if (c != 0) {
				char quotient[32], remainder[32];
				jive_bitstring_division_signed(quotient, remainder, rbits, cbits, 32);

				char div[32], mod[32];
				jive_bitstring_init_signed(div, 32, r/c);
				jive_bitstring_init_signed(mod, 32, r%c);

				assert(jive_bitstring_equal(quotient, div, 32) == '1');
				assert(jive_bitstring_equal(remainder, mod, 32) == '1');
			}
		}
	}

	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-bitstring", test_main);
