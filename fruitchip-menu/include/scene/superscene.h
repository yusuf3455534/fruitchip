#pragma once

#include <mlib/m-array.h>

#include <state.h>

typedef struct scene {
    void (*tick_handler)(struct state *state);
    void (*input_handler)(struct state *state, int input);
    void (*paint_handler)(struct state *state);
} scene_t;

void scene_init(scene_t *obj);

void scene_copy(scene_t *obj, const scene_t *src);

void scene_free(scene_t *obj);


ARRAY_DEF(
    array_scene,
    struct scene,
    (
        INIT(API_2(scene_init)),
        INIT_SET(API_6(scene_copy)),
        SET(API_6(scene_copy)),
        CLEAR(API_2(scene_free)),
    )
)

struct superscene {
    array_scene_t scenes;
    u64 last_paint;
};

void scene_tick_handler_superscene(struct state *state);
void scene_input_handler_superscene(struct state *state, int input);
void scene_paint_handler_superscene(struct state *state);
void superscene_paint_after_vsync(struct state *state);
void superscene_push_scene(scene_t scene);
void superscene_pop_scene();
void superscene_clear_button_guide(struct state *state);

