#include "encoding.h"

jive_encode_result
jive_i386_encode_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	DEBUG_ASSERT(r1 == outputs[0]->code);
	
	if (!jive_buffer_putbyte(target, icls->code)) return jive_encode_out_of_memory;
	if (!jive_buffer_putbyte(target, 0xc0|r1|(r2<<3))) return jive_encode_out_of_memory;
	
	return 0;
}

jive_encode_result
jive_i386_encode_regmove(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = outputs[0]->code;
	int r2 = inputs[0]->code;
	
	if (!jive_buffer_putbyte(target, icls->code)) return jive_encode_out_of_memory;
	if (!jive_buffer_putbyte(target, 0xc0|r1|(r2<<3))) return jive_encode_out_of_memory;
	
	return 0;
}

jive_encode_result
jive_i386_encode_mul_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int r1 = inputs[0]->code;
	int r2 = inputs[1]->code;
	
	DEBUG_ASSERT(r1 == outputs[0]->code);
	
	if (!jive_buffer_putbyte(target, 0x0f)) return jive_encode_out_of_memory;
	if (!jive_buffer_putbyte(target, 0xaf)) return jive_encode_out_of_memory;
	if (!jive_buffer_putbyte(target, 0xc0|r2|(r1<<3))) return jive_encode_out_of_memory;
	
	return 0;
}
