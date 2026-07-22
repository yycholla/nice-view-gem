#include <zephyr/kernel.h>
#include "battery.h"

/* Battery as an unlabeled thin line across the top of the (portrait)
 * screen: 2px tall, fills left-to-right, full charge spans the 68px
 * width. Drawn in the top canvas only. */
void draw_battery_line(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_rect_dsc_t rect_fg_dsc;
    init_rect_dsc(&rect_fg_dsc, LVGL_FOREGROUND);

    int len = (state->battery * BUFFER_SIZE) / 100;
    lv_canvas_draw_rect(canvas, 0, 0, len, 2, &rect_fg_dsc);

    /* charging: 1px full-width rail so the level line visibly sits on it */
    if (state->charging) {
        lv_canvas_draw_rect(canvas, 0, 0, BUFFER_SIZE, 1, &rect_fg_dsc);
    }
}
