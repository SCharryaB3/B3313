#ifndef PERSONALIZATION_HELPERS_H
#define PERSONALIZATION_HELPERS_H

// #include "engine/graph_node.h"
// #include "game/area.h"

s32 set_mario_animation_then_check_for_climbable_wall(struct MarioState *m, s32 animation);
s32 act_electric_idle(struct MarioState *m);
s32 set_mario_animation_then_check_for_climbable_wall(struct MarioState *m, s32 animation);
s32 act_squatkick(struct MarioState *m);
s32 act_climbing_wall(struct MarioState *m);

void hueRotateRGBA5551(u16 *ptr, int size, int hue);
void rgbMultiplyRGBA16(u16 *ptr, int size, float _r, float _g, float _b);
void texCopyRGBA16(u16 *ptrFrom, u16 *ptrTo, int size);
void darkenRGBA16(u16 *ptr, int size);
void rotateRGBA16(u16 *ptr, int size);
void invertRGBA16(u16 *ptr, int size, u8 lsd_texture_type);
void normalizeRGBA16(u16 *ptr, int size);

extern u32 gCurrHudType;

#endif
