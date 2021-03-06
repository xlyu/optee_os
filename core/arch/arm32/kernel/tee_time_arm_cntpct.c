/*
 * Copyright (c) 2014, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <kernel/misc.h>
#include <kernel/tee_time.h>
#include <kernel/tee_core_trace.h>
#include <kernel/time_source.h>
#include <mm/core_mmu.h>
#include <utee_defines.h>

#include <assert.h>
#include <stdint.h>
#include <mpa.h>

static uint32_t do_div(uint64_t *dividend, uint32_t divisor)
{
	mpa_word_t remainder = 0, n0, n1;
	n0 = (*dividend) & UINT_MAX;
	n1 = ((*dividend) >> WORD_SIZE) & UINT_MAX;
	*dividend = __mpa_div_dword(n0, n1, divisor, &remainder);
	return remainder;
}

static uint64_t read_cntpct(void)
{
	uint64_t val;
	uint32_t low, high;
	__asm__ volatile("mrrc	p15, 0, %0, %1, c14\n"
		: "=r"(low), "=r"(high)
		:
		: "memory");
	val = low | ((uint64_t)high << WORD_SIZE);
	return val;
}

static uint32_t read_cntfrq(void)
{
	uint32_t frq;
	__asm__ volatile("mrc	p15, 0, %0, c14, c0, 0\n"
		: "=r"(frq)
		:
		: "memory");
	return frq;
}

static TEE_Result arm_cntpct_get_sys_time(TEE_Time *time)
{
	uint64_t cntpct = read_cntpct();
	uint32_t cntfrq = read_cntfrq();
	uint32_t remainder;

	remainder = do_div(&cntpct, cntfrq);

	time->seconds = (uint32_t)cntpct;
	time->millis = remainder / (cntfrq / TEE_TIME_MILLIS_BASE);

	return TEE_SUCCESS;
}

static const struct time_source arm_cntpct_time_source = {
	.name = "arm cntpct",
	.get_sys_time = arm_cntpct_get_sys_time,
};

REGISTER_TIME_SOURCE(arm_cntpct_time_source)
