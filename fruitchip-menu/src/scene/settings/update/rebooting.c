#include <unistd.h>

#include <modchip/flash.h>

#include <components/progress_bar.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <scene/message.h>
#include <constants.h>
#include <utils.h>

static u32 countdown;

#define REBOOT_DEADLINE_SEC 30

static void scene_tick_handler_update_rebooting(struct state *state)
{
    enum modchip_reboot_mode reboot_mode;
    switch (state->update_type)
    {
        case UPDATE_TYPE_FW:
            reboot_mode = MODCHIP_REBOOT_MODE_UPDATE;
            break;
        case UPDATE_TYPE_APPS:
            reboot_mode = MODCHIP_REBOOT_MODE_SIMPLE;
            break;
    }

    u64 reboot_start = clock_us();
    u64 reboot_deadline = reboot_start + (1000000 * REBOOT_DEADLINE_SEC);
    if (!modchip_reboot_with_retry(reboot_mode, MODCHIP_CMD_RETRIES))
    {
        superscene_pop_scene();
        scene_switch_to_message(state, L"Failed to reboot the modchip");
        return;
    }

    // wait for reboot
    while (true)
    {
        if (modchip_ping()) break;

        state->repaint = true;
        scene_paint_handler_superscene(state);

        sleep(1);
        countdown -= 1;

        if (clock_us() > reboot_deadline)
        {
            superscene_pop_scene();
            scene_switch_to_message(state, L"Failed to reboot the modchip, timed out");
            return;
        }
    }

    superscene_pop_scene();
    scene_switch_to_update_complete();
    state->repaint = true;
}

static void scene_paint_handler_update_rebooting(struct state *state)
{
    float progress = (float)countdown / REBOOT_DEADLINE_SEC;
    progress_bar_paint_center_with_message(state->gs, progress, L"Rebooting modchip");

    superscene_clear_button_guide(state);
}

void scene_switch_to_update_rebooting()
{
    scene_t scene;
    scene_init(&scene);
    scene.tick_handler = scene_tick_handler_update_rebooting;
    scene.paint_handler = scene_paint_handler_update_rebooting;
    superscene_push_scene(scene);

    countdown = REBOOT_DEADLINE_SEC;
}