/*
 * Copyright (C) 2013 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>


#define ARGB_8888_MAX   0xFFFFFFFF
#define ARGB_8888_MIN   0x00000000
#define RGB_565_MAX 0xFFFF
#define RGB_565_MIN 0x0000

struct test_t
{
    char name[256];
    uint32_t dst_color;
    uint32_t src_color;
    size_t count;
};

struct test_t tests[] =
{
    {"Count 1, Src=Max, Dst=Min", ARGB_8888_MAX, RGB_565_MIN, 1},
    {"Count 2, Src=Min, Dst=Max", ARGB_8888_MIN, RGB_565_MAX, 2},
    {"Count 3, Src=Max, Dst=Max", ARGB_8888_MAX, RGB_565_MAX, 3},
    {"Count 4, Src=Min, Dst=Min", ARGB_8888_MAX, RGB_565_MAX, 4},
    {"Count 1, Src=Rand, Dst=Rand", 0x12345678, 0x9ABC, 1},
    {"Count 2, Src=Rand, Dst=Rand", 0xABCDEF12, 0x2345, 2},
    {"Count 3, Src=Rand, Dst=Rand", 0x11111111, 0xEDFE, 3},
    {"Count 4, Src=Rand, Dst=Rand", 0x12345678, 0x9ABC, 4},
    {"Count 5, Src=Rand, Dst=Rand", 0xEFEFFEFE, 0xFACC, 5},
    {"Count 10, Src=Rand, Dst=Rand", 0x12345678, 0x9ABC, 10}
};

void scanline_col32cb16blend_arm64(uint16_t *dst, int32_t src, size_t count);
void scanline_col32cb16blend_c(uint16_t * dst, int32_t src, size_t count)
{
    int srcAlpha = (src>>24);
    int f = 0x100 - (srcAlpha + (srcAlpha>>7));
    while (count--)
    {
        uint16_t d = *dst;
        int dstR = (d>>11)&0x1f;
        int dstG = (d>>5)&0x3f;
        int dstB = (d)&0x1f;
        int srcR = (src >> (   3))&0x1F;
        int srcG = (src >> ( 8+2))&0x3F;
        int srcB = (src >> (16+3))&0x1F;
        srcR += (f*dstR)>>8;
        srcG += (f*dstG)>>8;
        srcB += (f*dstB)>>8;
        *dst++ = (uint16_t)((srcR<<11)|(srcG<<5)|srcB);
    }
}

void scanline_col32cb16blend_test()
{
    uint16_t dst_c[16], dst_asm[16];
    uint32_t i, j;

    for(i = 0; i < sizeof(tests)/sizeof(struct test_t); ++i)
    {
        struct test_t test = tests[i];

        printf("Testing - %s:",test.name);

        memset(dst_c, 0, sizeof(dst_c));
        memset(dst_asm, 0, sizeof(dst_asm));

        for(j = 0; j < test.count; ++j)
        {
            dst_c[j]   = test.dst_color;
            dst_asm[j] = test.dst_color;
        }


        scanline_col32cb16blend_c(dst_c, test.src_color, test.count);
        scanline_col32cb16blend_arm64(dst_asm, test.src_color, test.count);


        if(memcmp(dst_c, dst_asm, sizeof(dst_c)) == 0)
            printf("Passed\n");
        else
            printf("Failed\n");

        for(j = 0; j < test.count; ++j)
        {
            printf("dst_c[%d] = %x, dst_asm[%d] = %x \n", j, dst_c[j], j, dst_asm[j]);
        }
    }
}

int main()
{
    scanline_col32cb16blend_test();
    return 0;
}
