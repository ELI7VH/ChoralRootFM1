/* SPDX-License-Identifier: GPL-3.0-only
 * Real native UI, session SAVE, and backup/restore path for WaveLoop SHAYPE. */
#define CR_SHAPE_EMBED 1
#include "cr_backup_test.c"
static void drain_controls(void){while(cr_evq_r!=cr_evq_w){cr_apply(&cr_evq[cr_evq_r%CR_EVQ]);cr_evq_r++;}}
int main(void)
{
    emu_flash_open();
    memset(emu_flash, 0xFF, sizeof emu_flash);
    cr_bank_boot();
    cr_settings_boot();
    cr_ui_init();
    usb.config = 1;
    cs.perf_sel = 5;
    cs.par[CR_PM_PATTERN][CR_P_PATTERN] = CR_PATTERN_SHAYPE;
    cr_set_perform_mode(&cr, CR_PM_PATTERN);
    cr_set_param(&cr, CR_PM_PATTERN, CR_P_PATTERN, CR_PATTERN_SHAYPE);
    cu.lock = L_PERF;
    cu_shape_page = 1;
    cu_shape_turn(0, 60);
    CHECK(cu_shape.steps == 64, "native Shape page edits all 64 steps");
    cu_oct_tap(B_OCTUP);
    CHECK(cu_shape_page == 2, "OCT selects the Draw page");
    cu_shape_step = 63;
    cu_shape_row = 7;
    cu_shape_len = 64;
    cu_shape_turn(3, 1);
    int at = cu_shape_cell();
    CHECK(at >= 0 && ARP_LEN(cu_shape.pat.note[at]) == 64, "native drawing adds row8 length64 at step64");
    CHECK(cr.shape.steps == 64 && cr.shape.pat.n == cu_shape.pat.n, "UI publishes a coherent engine pattern");
    cr_screen_t s;
    cr_screen_clear(&s);
    cu_perf_cells(&s);
    CHECK(!strcmp(s.cell[0][0].label, "Step") && !strcmp(s.cell[0][3].value, "On"),
          "native knob cells describe the selected note");
    cu_shape_page=1;cu_shape_turn(3,-2);cr_screen_clear(&s);cu_perf_cells(&s);
    CHECK(cu_shape.octave==-2 && cr.shape.octave==-2 && !strcmp(s.cell[0][3].label,"Octave"),"Shape K4 publishes signed octave and correct label");
    cu_shape_turn(3,-10);CHECK(cu_shape.octave==-2,"octave lower limit");
    cu_shape_turn(3,20);CHECK(cu_shape.octave==2,"octave upper limit");
    opt_set(O_LAYOUT,2);drain_controls();
    CHECK(cs.layout==2 && cr.scale_keys && opt_get(O_LAYOUT)==2,"Scale Keys option reaches native engine");
    char layout_name[32];opt_text(O_LAYOUT,layout_name,sizeof layout_name);
    CHECK(!strcmp(layout_name,"Scale Keys"),"third layout has explicit option label");
    cu.lock=0;cu_key_press(4);drain_controls();cu_key_release(4);drain_controls();
    CHECK(cr.voicing==2,"A3 selects second inversion and keeps it after release");
    cu_key_press(1);cu_key_press(3);cu_key_press(5);drain_controls();
    CHECK(cr.mods_down==(11u<<4),"black key combination reaches 7/9/13 extension set");
    cu_key_release(3);drain_controls();CHECK(cr.mods_down==(3u<<4),"releasing G sharp removes only the ninth");
    cu_key_release(1);cu_key_release(5);drain_controls();CHECK(!cr.mods_down,"released black keys leave no extensions held");
    cu.lock=L_PERF;
    CHECK(cr_settings_commit() == 0, "native global SAVE stores SHAYPE with the session");
    persist_t stored;
    CHECK(st_load(OBJ_SETTINGS,&stored,sizeof stored)==sizeof stored && CRS_LAYOUT(&stored.cr)==2u,"native global SAVE persists the Scale Keys layout bit");
    cr_session_t session;
    CHECK(st_load(OBJ_SESSION, &session, sizeof session) == CSS_SIZE &&
              cr_session_import(&session, &session, CSS_SIZE) == 1 && session.shape.steps == 64 && session.shape.octave == 2,
          "session storage loads the drawn pattern");
    CHECK(t_capture() == 0 && find(man, nman, 69) && find(man, nman, 69)->size == CSS_SIZE,
          "backup object69 contains the saved SHAYPE session");
    obj_t *o = find(man, nman, 69);
    CHECK(o && t_put(69, o->bytes, o->size) == 0, "validated session restore accepted");
    if (o) {
        o->bytes[0] ^= 1;
        CHECK(t_put(69, o->bytes, o->size) == 2, "another session magic is refused despite matching wire CRC");
    }
    cr_screen_clear(&s);
    s.kind=CR_K_KNOBROW;s.kr_band=3;s.shape=cu_shape;s.pos=0;
    cr_draw(&s,0);s.pos=1;cr_draw(&s,0);
    CHECK(cr_dc.drawn>0 && cr_dc.drawn<CR_NSTRIP,"moving SHAYPE playhead redraws only the grid band");
    cu_shape_page=0;cu_shape.preset=0;cu_shape_turn(0,-1);
    CHECK(!cu_shape_on() && cs.par[CR_PM_PATTERN][CR_P_PATTERN]==12,"turning past first starter returns to native patterns");
    cr_settings_revert();
    CHECK(cu_shape.steps == 64 && cu_shape.octave == 2, "native revert applies the saved SHAYPE pattern");
    printf("SHAYPE UI/storage: %u checks, %u failed\n", checks, fails);
    return fails != 0;
}
