#ifndef JIVE_I386_ENCODING_H
#define JIVE_I386_ENCODING_H

#include <stdint.h>

#include <jive/machine.h>
#include <jive/internal/instruction_str.h>
#include "debug.h"

static inline uint32_t
cpu_to_le32(uint32_t value)
{
	/* FIXME: endianness */
	return value;
}

static inline jive_encode_result
jive_i386_encode_simple(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	if (!jive_buffer_putbyte(target, icls->code))
		return jive_encode_out_of_memory;;
	return jive_encode_ok;
}

static inline jive_encode_result
jive_i386_encode_int_load_imm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	int reg = outputs[0]->code;
	if (!jive_buffer_putbyte(target, icls->code|reg)) return jive_encode_out_of_memory;
	uint32_t immediate = cpu_to_le32(immediates[0]);
	if (!jive_buffer_put(target, &immediate, 4)) return jive_encode_out_of_memory;
	return jive_encode_ok;
}

static inline jive_encode_result
jive_i386_r2i(const jive_cpureg * r1, const jive_cpureg * r2, uint32_t displacement, jive_buffer * target)
{
	int regcode = (r1->code) | (r2->code<<3);
	/* special treatment for load/store through ebp: always encode displacement parameter */
	bool code_displacement = displacement || (r1->code==5);
	bool large_displacement = (displacement>127) || (displacement<-128);
	bool success;
	if (code_displacement) {
		if (large_displacement) regcode |= 0x80;
		else regcode |= 0x40;
	}
	
	if (!jive_buffer_putbyte(target, regcode)) return jive_encode_out_of_memory;
	if (r1->code == 4) {
		/* esp special treatment */
		if (!jive_buffer_putbyte(target, 0x24)) return jive_encode_out_of_memory;
	}
	
	if (code_displacement) {
		if (large_displacement) {
			displacement = cpu_to_le32(displacement);
			success = jive_buffer_put(target, &displacement, 4);
		} else success = jive_buffer_putbyte(target, displacement);
	} else success = true;
	
	if (!success) return jive_encode_out_of_memory;
	return jive_encode_ok;
}

jive_encode_result
jive_i386_encode_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[]);

jive_encode_result
jive_i386_encode_regimm(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[]);

jive_encode_result
jive_i386_encode_regmove(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[]);

jive_encode_result
jive_i386_encode_mul_regreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[]);

jive_encode_result
jive_i386_encode_unaryreg(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[]);


#endif
