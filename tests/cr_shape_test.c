/* SPDX-License-Identifier: GPL-3.0-only
 * WaveLoop SHAYPE through ChoralRoot's actual note ownership and scheduler. */
#define CR_SHAPE_EMBED 1
#include "cr_engine_test.c"
#include "../firmware/src/cr_settings.h"
static void shape_start(void)
{
    cr_init(&C, &OUT);
    cr_tick(&C, now_ms());
    clear();
    MD(CR_MOD_MAJ);
    cr_set_perform_mode(&C, CR_PM_PATTERN);
    cr_set_param(&C, CR_PM_PATTERN, CR_P_PATTERN, CR_PATTERN_SHAYPE);
    cr_set_perform(&C, 1);
}
static void shape_end(void)
{
    cr_panic(&C);
    step(5);
    ok(!sounding() && !dup_on && !stray_off, "SHAYPE leaves no stuck notes, duplicate ons or stray offs");
}
int main(void)
{
    TICK_US = 1000;
    BASE_MS = 0;
    next_tick = 0;
    T = 0;
    shape_start();
    K(48, 1);
    step(260);
    ok(count('+', CR_STREAM_MAIN) == 2, "SHAYPE default starts two rows of the major chord");
    shape_end();
    int base[8], ids[8], bn;
    shape_start();K(48,1);step(260);bn=ons(ids,8,CR_STREAM_MAIN);
    for(int i=0;i<bn;i++)base[i]=LOG[ids[i]].note;
    shape_end();
    for(int oct=-2;oct<=2;oct++) {
        shape_start();C.shape.octave=(int8_t)oct;C.shape.root=100; /* legacy floor must not override octave */
        K(48,1);step(260);int n=ons(ids,8,CR_STREAM_MAIN),same=n==bn;
        for(int i=0;i<n&&i<bn;i++)if(LOG[ids[i]].note!=base[i]+12*oct)same=0;
        ok(same,"SHAYPE octave shifts every chord row down/up in exact octaves, ignoring legacy floor");shape_end();
    }
    shape_start();K(48,1);step(260);clear();C.shape.octave=-1;step(260);
    int changed=ons(ids,8,CR_STREAM_MAIN)>0;
    for(int i=0;i<ons(ids,8,CR_STREAM_MAIN);i++)if(LOG[ids[i]].note>=60)changed=0;
    ok(changed,"octave edit affects upcoming notes while the chord remains held");shape_end();
    shape_start();C.shape.octave=-2;K(0,1);step(260);
    ok(count('+',CR_STREAM_MAIN)==0,"downward octave drops pitches below MIDI zero without wrapping");shape_end();
    shape_start();C.shape.octave=2;K(120,1);step(260);
    ok(count('+',CR_STREAM_MAIN)==0,"upward octave drops pitches above MIDI127 without wrapping");shape_end();
    cr_shape_t saved;cr_shape_defaults(&saved);saved.octave=3;
    ok(!cr_shape_valid(&saved),"out-of-range octave rejected");
    for (int i = 0; i < CR_DIV_COUNT; i++) {
        unsigned id = cr_div_turn((unsigned)i, 0);
        ok(id == (unsigned)i, "division lookup preserves stored IDs");
    }
    const int rate[3] = {CR_DIV_1_64, CR_DIV_1_64T, CR_DIV_1_64D};
    const unsigned num[3] = {125, 125, 375}, den[3] = {4, 6, 8};
    for (int r = 0; r < 3; r++) {
        shape_start();
        C.shape.pat.n = 1;
        C.shape.pat.note[0] = ARP_NOTE(0, 0, 1);
        C.shape.steps = 1;
        cr_set_param(&C, CR_PM_PATTERN, CR_P_DIV, rate[r]);
        uint32_t epoch = cr_clock_epoch(&C);
        K(48, 1);
        step(1000);
        int idx[64], n = ons(idx, 64, CR_STREAM_MAIN);
        int exact = n >= 20;
        for (int j = 0; j < n; j++) {
            uint32_t due = epoch + (2u * (uint32_t)(j + 1) * num[r] + den[r]) / (2u * den[r]);
            if (LOG[idx[j]].ms - due > 1u)
                exact = 0;
        }
        ok(exact, "64th straight/triplet/dotted periods land on absolute rational grid");
        shape_end();
    }
    for (unsigned sw = 50; sw <= 90; sw += 40)
        for (int a = 0; a < CR_DIV_COUNT; a++)
            for (int b = 0; b < CR_DIV_COUNT; b++) {
                shape_start();
                cr_set_param(&C, CR_PM_PATTERN, CR_P_DIV, a);
                cr_set_param(&C, CR_PM_PATTERN, CR_P_SWING, (int)sw);
                K(48, 1);
                step(300);
                unsigned vi = 0;
                while (vi < CR_NVOICE && !C.v[vi].used)
                    vi++;
                unsigned pos = vi < CR_NVOICE ? C.v[vi].clk.pos : 0;
                cr_set_param(&C, CR_PM_PATTERN, CR_P_DIV, b);
                ok(vi < CR_NVOICE && C.v[vi].clk.pos == pos,
                   "rate change preserves pattern position and joins a future slot");
                shape_end();
            }
    for (unsigned d = 0; d <= AD_SHYFT; d++)
        for (unsigned p = 0; p < ARP_NFACTORY; p++) {
            shape_start();
            ash_load(&C.shape.pat, p);
            C.shape.steps = ARP_FACTORY[p].steps;
            C.shape.dir = (uint8_t)d;
            K(48, 1);
            step(2000);
            shape_end();
        }
    shape_start();
    C.shape.pat.n=1;C.shape.pat.note[0]=ARP_NOTE(0,0,1);C.shape.steps=1;
    cr_set_tempo(&C,70);cr_set_param(&C,CR_PM_PATTERN,CR_P_DIV,CR_DIV_1_1D);
    uint32_t epoch=cr_clock_epoch(&C);step(150000);clear();K(48,1);step(10000);
    int idx[16],n=ons(idx,16,CR_STREAM_MAIN),exact=n>0;
    for(int j=0;j<n;j++){
        uint32_t elapsed=LOG[idx[j]].ms-epoch;
        uint32_t slot=(elapsed*70u+180000u)/360000u;
        uint32_t due=epoch+(slot*360000u+35u)/70u;
        if(LOG[idx[j]].ms-due>1u)exact=0;
    }
    ok(exact,"dotted grid and new chord join remain exact after 48-beat global rebasing");shape_end();
    cr_session_t old, got;
    cr_session_defaults(&old);
    old.voicing = 5;
    old.version = 1;
    old.size = 1024;
    unsigned h = 2166136261u;
    unsigned char *bytes = (unsigned char *)&old;
    for (unsigned i = 12; i < 1024; i++)
        h = (h ^ bytes[i]) * 16777619u;
    old.check = h;
    ok(cr_session_import(&got, &old, 1024) == 2 && got.voicing == 5 && cr_shape_valid(&got.shape),
       "1024-byte upstream session imports original state and default SHAYPE");
    old = got;
    old.shape.steps = 64;
    old.shape.pat.n = 1;
    old.shape.pat.note[0] = ARP_NOTE(63, 7, 64);
    cr_session_seal(&old);
    ok(cr_session_import(&got, &old, sizeof old) == 1 && !memcmp(&old, &got, sizeof old),
       "extended session round-trips step64 row8 length64");
    printf("SHAYPE engine: %d passed, %d failed\n", passed, failed);
    return failed != 0;
}
