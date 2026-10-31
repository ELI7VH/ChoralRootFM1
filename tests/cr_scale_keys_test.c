/* SPDX-License-Identifier: GPL-3.0-only
 * WaveLoop Scale Keys: actual chord engine, diatonic extension sets and native ownership. */
#define CR_SHAPE_EMBED 1
#include "cr_engine_test.c"
#include "../firmware/src/cr_settings.h"
static const int DEG[2][7]={{0,2,4,5,7,9,11},{0,2,3,5,7,8,10}};
static void start_scale(int tonic,int scale){cr_init(&C,&OUT);cr_set_key(&C,0,(uint8_t)tonic,(cr_scale_t)scale);cr_set_scale_keys(&C,1);clear();}
static void end_scale(void){cr_panic(&C);step(5);ok(!sounding()&&!dup_on&&!stray_off,"Scale Keys releases every owned pitch cleanly");}
int main(void){TICK_US=1000;BASE_MS=0;next_tick=0;T=0;
 for(int scale=0;scale<2;scale++)for(int tonic=0;tonic<12;tonic++)for(int deg=0;deg<7;deg++)for(unsigned mask=0;mask<8;mask++)for(int inv=0;inv<4;inv++){
  start_scale(tonic,scale);cr_scale_controls(&C,inv,mask);int root=48+tonic+DEG[scale][deg];K(root,1);
  int hops[6]={0,2,4,6,8,12},expected[8],n=0;
  for(int j=0;j<6;j++)if(j<3 || ((mask>>(j-3))&1u)){
   int ix=deg+hops[j];expected[n++]=48+tonic+DEG[scale][ix%7]+12*(ix/7);
  }
  for(int j=0;j<inv;j++)expected[j%n]+=12*(j/n+1);
  for(int j=1;j<n;j++)for(int k=j;k&&expected[k-1]>expected[k];k--){int t=expected[k];expected[k]=expected[k-1];expected[k-1]=t;}
  char want[128]="",got[128];for(int j=0;j<n;j++)snprintf(want+strlen(want),8,j?" %d":"%d",expected[j]);
  on_notes_sorted(got,CR_STREAM_MAIN);ok(!strcmp(got,want),"all keys/scales, 7/9/13 combinations and inversions match diatonic pitches");end_scale();
 }
 start_scale(0,CR_SCALE_MAJOR);K(60,1);cr_scale_controls(&C,1,7);clear();cr_scale_controls(&C,2,3);cr_scale_controls(&C,-1,0);end_scale();
 start_scale(0,CR_SCALE_MAJOR);cr_set_perform_mode(&C,CR_PM_PATTERN);cr_set_param(&C,CR_PM_PATTERN,CR_P_PATTERN,CR_PATTERN_SHAYPE);cr_set_perform(&C,1);K(60,1);step(510);
 unsigned vi=0;while(vi<CR_NVOICE&&!C.v[vi].used)vi++;
 unsigned pos=C.v[vi].clk.pos;cr_scale_controls(&C,-1,7);
 ok(C.v[vi].clk.pos==pos,"extension combos preserve SHAYPE clock/pattern position");
 cr_scale_controls(&C,2,7);ok(C.v[vi].clk.pos==pos,"inversion key preserves SHAYPE melodic position");
 cr_scale_controls(&C,-1,7);ok(C.v[vi].clk.pos==pos,"unchanged held control does not retrigger SHAYPE");end_scale();
 cr_settings_t s,r;cr_settings_defaults(&s);s.raw_sound=(uint8_t)(CRS_RAW_BYTE(1,16)|CRS_SCALE_KEYS);cr_settings_seal(&s);
 ok(cr_settings_import(&r,&s,sizeof s)==1 && CRS_LAYOUT(&r)==2 && CRS_MIDI_KEYS(&r)==16 && CRS_RAW(&r),"Scale Keys saved bit round trips without changing RAW or MIDI Keys");
 cr_settings_out_t routing;cr_settings_apply(&r,&C,&routing);
 ok(C.scale_keys,"saved Scale Keys selection reaches the engine");
 printf("Scale Keys: %d passed, %d failed\n",passed,failed);return failed!=0;}
