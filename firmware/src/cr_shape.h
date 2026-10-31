/* SPDX-License-Identifier: GPL-3.0-only
 * WaveLoop SHAYPE adapter data; no hardware dependency. */
#ifndef CR_SHAPE_H
#define CR_SHAPE_H
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "arp_shape.c"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
typedef struct {
    arp_pat_t pat;
    uint8_t steps, dir, bound, preset;
    int8_t root; /* legacy pitch floor, retained for saved-record layout; no longer used */
    int8_t octave; /* -2..+2; formerly reserved, zero in older sessions */
} cr_shape_t;
void cr_shape_defaults(cr_shape_t *s);
int cr_shape_valid(const cr_shape_t *s);
#endif
