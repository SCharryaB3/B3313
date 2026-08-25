#include <PR/ultratypes.h>
#include <ultra64.h>
#include "sm64.h"
// #include "stats_tracking.h"
// #include "personalization_helpers.h"
#include "game/main.h"


#include "behavior_data.h"
#include "audio/external.h"
#include "geo_commands.h"
#include "game/camera.h"
// #include "game/decompress.h"
#include "game/game_init.h"
#include "game/geo_misc.h"
#include "game/hud.h"
#include "game/ingame_menu.h"
#include "game/interaction.h"
#include "game/level_geo.h"
#include "game/level_update.h"
#include "game/mario.h"
#include "game/mario_step.h"
#include "game/moving_texture.h"
#include "game/screen_transition.h"
#include "game/object_helpers.h"
#include "game/object_list_processor.h"
#include "game/obj_behaviors_2.h"
#include "game/paintings.h"
#include "game/save_file.h"
#include "surface_terrains.h"
#include "moving_texture_macros.h"
#include "level_misc_macros.h"
#include "macro_preset_names.h"
#include "special_preset_names.h"
#include "textures.h"
#include "dialog_ids.h"
#include "seq_ids.h"
#include "engine/behavior_script.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "segment_symbols.h"

#include "make_const_nonconst.h"

// for the OS code
// #include "lib/src/libultra_internal.h"


s32 act_electric_idle(struct MarioState *m) {
    play_sound_if_no_flag(m, SOUND_MARIO_ATTACKED, MARIO_ACTION_SOUND_PLAYED);
    play_sound(SOUND_MOVING_SHOCKED, m->marioObj->header.gfx.cameraToObject);
    set_camera_shake_from_hit(SHAKE_SHOCK);
    if (m->actionTimer == 0 || set_mario_animation(m, MARIO_ANIM_SHOCKED) == 0) {
        m->actionTimer++;
        m->flags |= MARIO_METAL_SHOCK;
    }

    if (m->actionTimer >= 4) {
        m->invincTimer = 30;
        set_mario_action(m, m->health < 0x0100 ? ACT_ELECTROCUTION : ACT_IDLE, 0);
    }
    stationary_ground_step(m);
    return FALSE;
}