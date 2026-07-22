#include <math.h>
#include <zephyr/kernel.h>
#include "activity.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(bongo_casualleft);
LV_IMG_DECLARE(bongo_casualright);
LV_IMG_DECLARE(bongo_furiousup);
LV_IMG_DECLARE(bongo_furiousdown);

/* Middle canvas (virtual y 44..112): held-modifier indicators and the
 * leader-key readout, replacing the WPM gauge/chart. */

#define MODS_Y (48 + BUFFER_OFFSET_MIDDLE)
#define LDR_Y (68 + BUFFER_OFFSET_MIDDLE)
#define CAND_Y1 (84 + BUFFER_OFFSET_MIDDLE)
#define CAND_Y2 (98 + BUFFER_OFFSET_MIDDLE)

static const char mod_letters[4] = {'S', 'C', 'A', 'G'};
/* HID mods byte: bit0 LCTRL, 1 LSHFT, 2 LALT, 3 LGUI (right variants +4) */
static const uint8_t mod_masks[4] = {0x22, 0x11, 0x44, 0x88};

static void draw_mods(lv_obj_t *canvas, const struct status_state *state) {
    /* blank unless something is held; fixed slots so glances stay learnable */
    if (state->mods == 0) {
        return;
    }
    lv_draw_label_dsc_t fg_text;
    init_label_dsc(&fg_text, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_CENTER);
    lv_draw_label_dsc_t bg_text;
    init_label_dsc(&bg_text, LVGL_BACKGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_CENTER);
    lv_draw_rect_dsc_t fg_rect;
    init_rect_dsc(&fg_rect, LVGL_FOREGROUND);

    char letter[2] = {0};
    for (int i = 0; i < 4; i++) {
        int x = 2 + i * 17;
        letter[0] = mod_letters[i];
        if (state->mods & mod_masks[i]) {
            lv_canvas_draw_rect(canvas, x - 1, MODS_Y - 1, 15, 13, &fg_rect);
            lv_canvas_draw_text(canvas, x, MODS_Y, 13, &bg_text, letter);
        } else {
            lv_canvas_draw_text(canvas, x, MODS_Y, 13, &fg_text, letter);
        }
    }
}

static void draw_leader(lv_obj_t *canvas, const struct status_state *state) {
    if (!state->leader_active) {
        return;
    }

    lv_draw_label_dsc_t fg_left;
    init_label_dsc(&fg_left, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_LEFT);
    lv_draw_rect_dsc_t fg_rect;
    init_rect_dsc(&fg_rect, LVGL_FOREGROUND);

    lv_canvas_draw_text(canvas, 0, LDR_Y, 34, &fg_left, "LDR");

    /* one dot per key already pressed in the sequence */
    for (int i = 0; i < state->leader_count && i < 5; i++) {
        lv_canvas_draw_rect(canvas, 36 + i * 6, LDR_Y + 4, 3, 3, &fg_rect);
    }

    /* candidate next keys, packed, up to two rows of six */
    int n = state->leader_cand_len;
    int first = n > 6 ? 6 : n;
    char row[8] = {0};
    memcpy(row, state->leader_cands, first);
    lv_canvas_draw_text(canvas, 0, CAND_Y1, 68, &fg_left, row);

    if (n > 6) {
        int second = (n - 6) > 6 ? 6 : (n - 6);
        memset(row, 0, sizeof(row));
        memcpy(row, &state->leader_cands[6], second);
        lv_canvas_draw_text(canvas, 0, CAND_Y2, 68, &fg_left, row);
    }
}

/* idle art: a little procedural sea urchin, body + spikes, no assets */
static void draw_urchin(lv_obj_t *canvas) {
    int cx = 34;
    int cy = 86 + BUFFER_OFFSET_MIDDLE;
    int body_r = 9;

    lv_draw_rect_dsc_t body_dsc;
    init_rect_dsc(&body_dsc, LVGL_FOREGROUND);
    body_dsc.radius = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(canvas, cx - body_r, cy - body_r, body_r * 2, body_r * 2, &body_dsc);

    lv_draw_line_dsc_t spike_dsc;
    init_line_dsc(&spike_dsc, LVGL_FOREGROUND, 1);
    for (int i = 0; i < 16; i++) {
        float a = i * (2.0f * 3.14159f / 16.0f);
        int len = (i % 2) ? 16 : 11; /* alternating long/short spikes */
        lv_point_t pts[2] = {
            {cx + (int)((body_r - 2) * cosf(a)), cy + (int)((body_r - 2) * sinf(a))},
            {cx + (int)(len * cosf(a)), cy + (int)(len * sinf(a))},
        };
        lv_canvas_draw_line(canvas, pts, 2, &spike_dsc);
    }

    /* eye-catch highlight */
    lv_draw_rect_dsc_t bg_dsc;
    init_rect_dsc(&bg_dsc, LVGL_BACKGROUND);
    lv_canvas_draw_rect(canvas, cx - 4, cy - 4, 2, 2, &bg_dsc);
}

/* bongo cat taps in sync with real keypresses; WPM only sets intensity */
static void draw_bongo(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);

    const lv_img_dsc_t *frame;
    if (state->wpm >= 60) {
        frame = state->bongo_paw ? &bongo_furiousup : &bongo_furiousdown;
    } else {
        frame = state->bongo_paw ? &bongo_casualleft : &bongo_casualright;
    }
    lv_canvas_draw_img(canvas, 0, 66 + BUFFER_OFFSET_MIDDLE, frame, &img_dsc);
}

void draw_activity_status(lv_obj_t *canvas, const struct status_state *state) {
    if (state->leader_active) {
        draw_leader(canvas, state);
    } else if (state->mods != 0) {
        draw_mods(canvas, state);
    } else if (state->wpm > 0) {
        draw_bongo(canvas, state);
    } else {
        draw_urchin(canvas);
    }
}
