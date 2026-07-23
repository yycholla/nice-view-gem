#include <math.h>
#include <zephyr/kernel.h>
#include "activity.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(bongo_casualleft);
LV_IMG_DECLARE(bongo_casualright);
LV_IMG_DECLARE(bongo_furiousup);
LV_IMG_DECLARE(bongo_furiousdown);
LV_IMG_DECLARE(bongo_resting);

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

LV_FONT_DECLARE(lv_font_unscii_8);

struct ldr_item {
    char key;
    const char *rest;
};

static const struct ldr_item top_items[] = {{'V', "iew"}, {'M', "ove"}, {'T', "ogl"},
                                            {'G', "rp"},  {'J', "j"},   {'C', "lip"},
                                            {'S', "ys"},  {'W', "in"},  {'P', "ad"}};
static const struct ldr_item jj_items[] = {
    {'S', "tat"}, {'L', "og"}, {'D', "iff"}, {'N', "ew"}, {'M', "sg"}};
static const struct ldr_item clip_items[] = {{'Y', "ank"}, {'P', "aste"}};
static const struct ldr_item sys_items[] = {{'C', "aff"},  {'D', "nd"},   {'N', "ite"},
                                            {'E', "moji"}, {'T', "heme"}, {'L', "ock"}};
static const struct ldr_item win_items[] = {{'O', "ver"}, {'Z', "oom"}, {'M', "ax"}};
static const struct ldr_item pad_items[] = {
    {'K', "itty"}, {'E', "macs"}, {'S', "pot"}, {'D', "isc"}};

/* one menu row: inverted key cell, then the rest of the word */
static void draw_ldr_item(lv_obj_t *canvas, int x, int y, const struct ldr_item *item) {
    lv_draw_rect_dsc_t fg_rect;
    init_rect_dsc(&fg_rect, LVGL_FOREGROUND);
    lv_draw_label_dsc_t key_text;
    init_label_dsc(&key_text, LVGL_BACKGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT);
    lv_draw_label_dsc_t rest_text;
    init_label_dsc(&rest_text, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT);

    char key[2] = {item->key, 0};
    lv_canvas_draw_rect(canvas, x, y - 1, 9, 10, &fg_rect);
    lv_canvas_draw_text(canvas, x + 1, y, 8, &key_text, key);
    lv_canvas_draw_text(canvas, x + 10, y, 33, &rest_text, item->rest);
}

static void draw_ldr_hint(lv_obj_t *canvas, int y, const char *text) {
    lv_draw_label_dsc_t fg_text;
    init_label_dsc(&fg_text, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT);
    lv_canvas_draw_text(canvas, 0, y, 68, &fg_text, text);
}

static void draw_leader(lv_obj_t *canvas, const struct status_state *state) {
    if (!state->leader_active) {
        return;
    }

    /* header: what this prefix is for */
    const char *header = "LEADER";
    const struct ldr_item *items = NULL;
    int n = 0;
    bool tag_grid = false;

    switch (state->leader_prefix) {
    case 'J':
        header = "JJ";
        items = jj_items;
        n = 5;
        break;
    case 'C':
        header = "CLIP";
        items = clip_items;
        n = 2;
        break;
    case 'S':
        header = "SYSTEM";
        items = sys_items;
        n = 6;
        break;
    case 'W':
        header = "WINDOW";
        items = win_items;
        n = 3;
        break;
    case 'P':
        header = "PADS";
        items = pad_items;
        n = 4;
        break;
    case 'V':
        header = "VIEW TAG";
        tag_grid = true;
        break;
    case 'M':
        header = "MOVE TAG";
        tag_grid = true;
        break;
    case 'T':
        header = "TOGL VIEW";
        tag_grid = true;
        break;
    case 'G':
        header = "TAG GRP";
        tag_grid = true;
        break;
    default:
        items = top_items;
        n = 9;
        break;
    }

    lv_draw_label_dsc_t hdr_text;
    init_label_dsc(&hdr_text, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_LEFT);
    lv_canvas_draw_text(canvas, 0, 44 + BUFFER_OFFSET_MIDDLE, 68, &hdr_text, header);

    int y0 = 58 + BUFFER_OFFSET_MIDDLE;
    if (tag_grid) {
        /* tag number grid reminder, calculator order */
        draw_ldr_hint(canvas, y0, "789 UIO");
        draw_ldr_hint(canvas, y0 + 10, "456 JKL");
        draw_ldr_hint(canvas, y0 + 20, "123 M,.");
    } else if (n > 6) {
        /* top level: two columns of five */
        for (int i = 0; i < n; i++) {
            draw_ldr_item(canvas, (i / 5) * 36, y0 + (i % 5) * 11, &items[i]);
        }
    } else {
        for (int i = 0; i < n; i++) {
            draw_ldr_item(canvas, 0, y0 + i * 9, &items[i]);
        }
    }
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
        /* idle: cat asleep at its desk */
        lv_draw_img_dsc_t img_dsc;
        lv_draw_img_dsc_init(&img_dsc);
        lv_canvas_draw_img(canvas, 0, 66 + BUFFER_OFFSET_MIDDLE, &bongo_resting, &img_dsc);
    }
}
