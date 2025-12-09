#include <stdio.h>

#include "components/font.h"
#include "scene/boot_list.h"
#include "scene/init.h"
#include "constants.h"

static void scene_input_handler_startup(struct state *state, int input)
{
    (void)state;
    (void)input;
}

static void scene_paint_handler_startup(struct state *state)
{
    font_print_centered(
        state->gs,
        (state->gs->Height / 2.0) - 8.0,
        1.0,
        FG,
        L"Initializing"
    );

    if (input_pad_is_ready())
    {
        superscene_pop_scene();
        scene_switch_to_boot_list(state);
    }
    else
    {
        state->repaint = true;
    }
}

void scene_switch_to_init(struct state *state)
{
    scene_t scene;
    scene_init(&scene);
    scene.input_handler = scene_input_handler_startup;
    scene.paint_handler = scene_paint_handler_startup;
    superscene_push_scene(scene);
    state->repaint = true;
}
