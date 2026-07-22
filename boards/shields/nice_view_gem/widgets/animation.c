#include <stdlib.h>
#include <zephyr/kernel.h>
#include "animation.h"

/* "Corro" sea urchin frames from GPeye/urchin-peripheral-animation (MIT) */
LV_IMG_DECLARE(corro01);
LV_IMG_DECLARE(corro02);
LV_IMG_DECLARE(corro03);
LV_IMG_DECLARE(corro04);
LV_IMG_DECLARE(corro05);
LV_IMG_DECLARE(corro06);
LV_IMG_DECLARE(corro07);
LV_IMG_DECLARE(corro08);
LV_IMG_DECLARE(corro09);
LV_IMG_DECLARE(corro10);
LV_IMG_DECLARE(corro11);
LV_IMG_DECLARE(corro12);

const lv_img_dsc_t *anim_imgs[] = {
    &corro01, &corro02, &corro03, &corro04, &corro05, &corro06, &corro07, &corro08, &corro09, &corro10, &corro11, &corro12,
};

void draw_animation(lv_obj_t *canvas) {
#if IS_ENABLED(CONFIG_NICE_VIEW_GEM_ANIMATION)
    lv_obj_t *art = lv_animimg_create(canvas);
    lv_obj_center(art);

    lv_animimg_set_src(art, (const void **)anim_imgs, 12);
    lv_animimg_set_duration(art, CONFIG_NICE_VIEW_GEM_ANIMATION_MS);
    lv_animimg_set_repeat_count(art, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(art);
#else
    lv_obj_t *art = lv_img_create(canvas);

    int length = sizeof(anim_imgs) / sizeof(anim_imgs[0]);
    srand(k_uptime_get_32());
    int random_index = rand() % length;

    lv_img_set_src(art, anim_imgs[random_index]);
#endif

    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 0, 0);
}
