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


// AI OBJ
u32 gCurrHudType = 0;
//TODO: ADD OTHER VARS


// so what? no functions there.

s32 act_squatkick(struct MarioState *m) {
	// m->actionState should be zero by default

	// set_mario_animation (m, MARIO_ANIM_SQUATKICKING); // TODO: add anim

	if (m->actionState == 0) 
	{
		//mario_set_forward_vel(m, 20.0f);
		if (m->actionTimer == 0)
		{
			m->forwardVel += 25.0f;
			m->actionTimer = 1;
		}
		m->vel[1] += 8.0f;
		if (m->marioObj->header.gfx.animInfo.animFrame >= 2)
			perform_air_step(m, 0);
		if (m->marioObj->header.gfx.animInfo.animFrame >= 3)
            m->actionState++;
		play_mario_sound(m, SOUND_ACTION_TERRAIN_JUMP, 0);
	}
	else 
	{
		switch (perform_air_step(m, 0))
		{
			case AIR_STEP_HIT_LAVA_WALL:
				lava_boost_on_wall(m);
			case AIR_STEP_HIT_WALL:
				//mario_set_forward_vel(m, 0.0f);
				// we should bonk
                mario_set_forward_vel(m, -8.0f);
                return set_mario_action(m, ACT_SOFT_BONK, 0);
				
				break;
				
			case AIR_STEP_NONE: 
				if (m->actionState == 1)
				{
					m->flags |= MARIO_KICKING;
					update_air_without_turn(m);
					if (is_anim_past_end(m)) m->actionState++;
				}
				else if (m->actionState == 2)
				{
					update_air_without_turn(m);
				}
				break;
			case AIR_STEP_LANDED: 
				set_mario_action(m, ACT_BUTT_SLIDE, 0);
				play_mario_landing_sound(m, SOUND_ACTION_TERRAIN_LANDING);
				break;
		}
	}
	return 0;
}

s32 set_mario_animation_then_check_for_climbable_wall(struct MarioState *m, s32 animation) {
    set_mario_animation(m, animation);

    if (m->wall) {
        if (m->wall->type == SURFACE_HANGABLE) {
            s16 wallDYaw = atan2s(m->wall->normal.z, m->wall->normal.x);
            m->faceAngle[1] = wallDYaw;
            return set_mario_action(m, ACT_CLIMBING_WALL, 0);
        }
    }
    
    return 0;
}

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
