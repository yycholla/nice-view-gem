#include <zephyr/kernel.h>
#include "battery.h"

/* Battery as an unlabeled thin line along the top edge of the landscape
 * screen (portrait x=0 column, which the 90deg CW canvas rotation maps to
 * the top row). Fills left-to-right: full charge spans the whole 160px
 * edge. Each canvas draws its own slice; LVGL clips the overflow. */
void draw_battery_line(lv_obj_t *canvas, const struct status_state *state, int offset,
                       int edge_len) {
    lv_draw_rect_dsc_t rect_fg_dsc;
    init_rect_dsc(&rect_fg_dsc, LVGL_FOREGROUND);

    int len = (state->battery * edge_len) / 100;
    /* left edge of landscape = high virtual y; grow toward the right */
    int y_start = edge_len - len;

    lv_canvas_draw_rect(canvas, 0, y_start + offset, 2, len, &rect_fg_dsc);

    /* charging: 1px full-length rail so the level line visibly sits on it */
    if (state->charging) {
        lv_canvas_draw_rect(canvas, 0, 0 + offset, 1, edge_len, &rect_fg_dsc);
    }
}
