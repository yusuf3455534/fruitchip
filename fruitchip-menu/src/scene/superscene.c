#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "scene/superscene.h"
#include "utils.h"
#include "state.h"
#include "constants.h"
#include "components/font.h"

void scene_init(scene_t *obj)
{
    obj->tick_handler = NULL;
    obj->input_handler = NULL;
    obj->paint_handler = NULL;
}

void scene_copy(scene_t *obj, const scene_t *src)
{
    scene_init(obj);

    if (src->tick_handler)
        obj->tick_handler = src->tick_handler;

    if (src->input_handler)
        obj->input_handler = src->input_handler;

    if (src->paint_handler)
        obj->paint_handler = src->paint_handler;
}

void scene_free(scene_t *obj)
{
    (void)obj;
}

struct superscene superscene = { 0 };

void scene_tick_handler_superscene(struct state *state)
{
    if (array_scene_empty_p(superscene.scenes))
        return;

    scene_t *top_scene = array_scene_back(superscene.scenes);

    if (top_scene->tick_handler)
        top_scene->tick_handler(state);
}

void scene_input_handler_superscene(struct state *state, int input)
{
    if (array_scene_empty_p(superscene.scenes))
        return;

    scene_t *top_scene = array_scene_back(superscene.scenes);

    if (top_scene->input_handler)
        top_scene->input_handler(state, input);
}

static void draw_header(struct state *state)
{
    if (!state->header)
        return;

    float y1 = HEADER_Y_START;
    float y2 = y1 + ITEM_H;

    // clear
    gsKit_prim_sprite(
        state->gs,
        MARGIN_X, y1,
        state->gs->Width - MARGIN_X, y2,
        1,
        BG
    );

    font_print(state->gs, ITEM_TEXT_X_START, y1 + ITEM_TEXT_Y_OFFSET, 1, FG, state->header);

    // draw separator line on top of the first list item
    gsKit_prim_sprite(
        state->gs,
        MARGIN_X, y2,
        state->gs->Width - MARGIN_X, y2 + HEADER_H,
        1,
        FG
    );
}

static void superscene_clear(struct state *state)
{
    gsKit_clear(state->gs, BG);
}

static void superscene_paint(struct state *state)
{
    gsKit_queue_exec(state->gs);
    gsKit_sync_flip(state->gs);
    gsKit_TexManager_nextFrame(state->gs);
    superscene.last_paint = clock_us();
}

void superscene_paint_after_vsync(struct state *state)
{
    u64 elapsed = clock_us() - superscene.last_paint;
    u32 target_framerate_us = state->gs->Mode == GS_MODE_PAL
        ? 20000 // 50 FPS, 20ms
        : 16667; // 60 FPS, 16.66ms

    if (elapsed > target_framerate_us)
    {
        state->repaint = true;
        scene_paint_handler_superscene(state);
    }
}

void scene_paint_handler_superscene(struct state *state)
{
    if (!state->repaint)
        return;

    // allow paint handler to request another repaint
    state->repaint = false;

    superscene_clear(state);

    if (!array_scene_empty_p(superscene.scenes))
    {
        scene_t *top_scene = array_scene_back(superscene.scenes);

        if (top_scene->paint_handler)
            top_scene->paint_handler(state);
    }

    draw_header(state);
    button_guide_draw(state->gs, &state->button_guide);
    superscene_paint(state);
}

void superscene_push_scene(scene_t scene)
{
    array_scene_push_back(superscene.scenes, scene);
}

void superscene_pop_scene()
{
    array_scene_pop_back(NULL, superscene.scenes);
}

void superscene_clear_button_guide(struct state *state)
{
    memset(&state->button_guide, 0, sizeof(state->button_guide));
}
