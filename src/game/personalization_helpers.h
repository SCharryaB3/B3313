#ifndef PERSONALIZATION_HELPERS_H
#define PERSONALIZATION_HELPERS_H

// #include "engine/graph_node.h"
// #include "game/area.h"

s32 set_mario_animation_then_check_for_climbable_wall(struct MarioState *m, s32 animation);
s32 act_electric_idle(struct MarioState *m);

extern u32 gCurrHudType;

#endif
