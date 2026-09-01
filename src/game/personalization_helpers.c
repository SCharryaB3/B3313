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


// HUE SHIFT THINGY
typedef struct RgbColor
{
	u8 r;
	u8 g;
	u8 b;
} RgbColor;

typedef struct HsvColor
{
	u8 h;
	u8 s;
	u8 v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    u8 region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    u8 rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * ((float)rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}


void hueRotateRGBA5551(u16 *ptr, int size, int hue) {
	int i;
	RgbColor rgbColor;
	HsvColor hsvColor;
	u16 value;
	
	for (i = 0; i < size; i++) {
		value = *ptr;
		
		rgbColor.r = (value >> 11) << 3;
		rgbColor.g = ((value >> 6) & 0x1F) << 3;
		rgbColor.b = ((value >> 1) & 0x1F) << 3;
		
		hsvColor = RgbToHsv(rgbColor);
		hsvColor.h = (u8)(hsvColor.h + hue);
		rgbColor = HsvToRgb(hsvColor);
		
		*ptr = (
			// R
			((rgbColor.r >> 3) << 11)
			// G
			+ ((rgbColor.g >> 3) << 6)
			// B
			+ ((rgbColor.b >> 3) << 1)
			// A
			+ (value & 1)
		);
		
		ptr++;
	}
}

void rgbMultiplyRGBA16(u16 *ptr, int size, float _r, float _g, float _b) {
	int i;
	RgbColor rgbColor;
	u16 value;
	
	for (i = 0; i < size; i++) {
		value = *ptr;
		
		rgbColor.r = (value >> 11) << 3;
		rgbColor.g = ((value >> 6) & 0x1F) << 3;
		rgbColor.b = ((value >> 1) & 0x1F) << 3;
		
        rgbColor.r *= _r;
        rgbColor.g *= _g;
        rgbColor.b *= _b;
		
		*ptr = (
			// R
			((rgbColor.r >> 3) << 11)
			// G
			+ ((rgbColor.g >> 3) << 6)
			// B
			+ ((rgbColor.b >> 3) << 1)
			// A
			+ (value & 1)
		);
		
		ptr++;
	}
}

void texCopyRGBA16(u16 *ptrFrom, u16 *ptrTo, int size) {
	int i;
	
	for (i = 0; i < size; i++) {
		*ptrTo = *ptrFrom;
        
        ptrFrom++;
        ptrTo++;
	}
}

u32 modelLoadedBits[8];

s32 get_model_loaded(u8 model) {
    return modelLoadedBits[model >> 5] & (1 << (model & 0x1F));
}

void set_model_loaded(u8 model, s32 isLoaded) {
    if (isLoaded) {
        modelLoadedBits[model >> 5] |= (1 << (model & 0x1F));
    }
    else {
        modelLoadedBits[model >> 5] &= ~(1 << (model & 0x1F));
    }
}

void clear_model_loaded() {
    s32 i;
    
    for (i = 0; i < 8; i++) {
        modelLoadedBits[i] = 0;
    }
}

// Climbma
// original code by iProgramInCpp (2021)
// adapted by a benedani for bee (2023)
// ported by timmy6259 for bee decomport (2026)
// trolls to get this function to be used are in Mario directory
f32 prevControlStickY = 0;
s32 act_climbing_wall(struct MarioState *m) {
	s16 wallDYaw, ang, xm, ym;
	if (m->input & INPUT_A_PRESSED) {
		m->vel[1] = 52.0f;
		m->faceAngle[1] += 0x8000;
		return set_mario_action(m, ACT_WALL_KICK_AIR, 0);
	}
	
	if (m->wall == NULL || m->wall->type != SURFACE_HANGABLE) {
        mario_set_forward_vel(m, 0.0f);
        return set_mario_action(m, ACT_FREEFALL, 0);
    }
    
    m->marioObj->header.gfx.animInfo.animAccel = 0;
    wallDYaw = atan2s(m->wall->normal.z, m->wall->normal.x);
    m->faceAngle[1] = 0X8000 + wallDYaw;
    m->marioObj->header.gfx.angle[1] = 0X8000 + wallDYaw;
    mario_set_forward_vel(m, 10.0f);
    
    if (m->marioObj->header.gfx.animInfo.animID < MARIO_ANIM_CLIMBING ||
        m->marioObj->header.gfx.animInfo.animID > MARIO_ANIM_CLIMBING_RIGHT) {
        set_mario_animation(m, MARIO_ANIM_CLIMBING);
        m->marioObj->header.gfx.animInfo.animAccel = 0;
        m->marioObj->header.gfx.animInfo.animFrame = 0;
    }
    
    if (m->controller->stickY == 0) {
        set_mario_animation(m, MARIO_ANIM_CLIMBING_DOWN);
        //set_mario_anim_with_accel(m, MARIO_ANIM_CLIMBING_DOWN,  0x10000 * m->vel[1] * 12);
        m->marioObj->header.gfx.animInfo.animFrame = 10;
    }
    if (m->input & INPUT_NONZERO_ANALOG) {
        //stick X - left/right
        //stick Y - up/down
        if (m->controller->stickY > 16.f || m->controller->stickY < -16.f) {
            m->vel[1] = m->controller->stickY * .25f;
            // set anim speed
            if (m->controller->stickY > 16.f) {
                set_mario_animation(m, MARIO_ANIM_CLIMBING);
                m->vel[1] *= .50f;
                m->marioObj->header.gfx.animInfo.animAccel = 0x40000;
            } else {
                m->vel[1] *= 1.0f;
                set_mario_animation(m, MARIO_ANIM_CLIMBING_DOWN);
                m->marioObj->header.gfx.animInfo.animFrame = 0;
                m->particleFlags |= PARTICLE_DUST;
                play_sound(SOUND_MOVING_TERRAIN_SLIDE, m->marioObj->header.gfx.cameraToObject);
                
            }
        } else { 
            m->particleFlags &= ~PARTICLE_DUST;
            m->vel[1] = 0; 
            if (m->marioObj->header.gfx.animInfo.animID != MARIO_ANIM_CLIMBING_DOWN) {
                set_mario_animation(m, MARIO_ANIM_CLIMBING_DOWN);
                m->marioObj->header.gfx.animInfo.animFrame = 10;
            }
        }
        if (m->controller->stickX > 16.f || m->controller->stickX < -16.f) {
            //m->vel[1] = m->controller->stickY * .5f;
            ang = m->faceAngle[1];
            // Calculate the X and Y would-be movements based on the angle
            if (m->controller->stickX > 16.f) {
                // going right
                ang -= 0x2000;
                xm = (s16)(8.f * sins(ang));
                ym = (s16)(8.f * coss(ang));
                // X and Y movement
                m->vel[0] = xm;
                m->vel[2] = ym;
                
                set_mario_animation(m, MARIO_ANIM_CLIMBING_RIGHT);
                m->marioObj->header.gfx.animInfo.animAccel = 0x5000 * 12;
            } else {
                // going left
                ang += 0x2000;
                xm = (s16)(8.f * sins(ang));
                ym = (s16)(8.f * coss(ang));
                // X and Y movement
                m->vel[0] = xm;
                m->vel[2] = ym;
                set_mario_animation(m, MARIO_ANIM_CLIMBING_LEFT);
                m->marioObj->header.gfx.animInfo.animAccel = 0x5000 * 12;
            }
        } else { 
            if (m->marioObj->header.gfx.animInfo.animID == MARIO_ANIM_CLIMBING) 
                ;//m->marioObj->header.gfx.animInfo.animAccel = 0x10000;
            else
                m->marioObj->header.gfx.animInfo.animAccel = 0;
            m->vel[0] = 0; 
            m->vel[2] = 0; 
        }
    } else { 
        m->vel[1] = 0; 
        m->vel[0] = 0;
        m->vel[2] = 0;
    }
    
    switch (perform_air_step(m, 0)) {
        case AIR_STEP_LANDED:
            mario_set_forward_vel(m, 0.0f);
            if (!check_fall_damage_or_get_stuck(m, ACT_HARD_BACKWARD_GROUND_KB)) {
                return set_mario_action(m, ACT_FREEFALL_LAND, 0);
            }
            break;
    }
    ang = m->faceAngle[1];
    xm = (s16)(50.f * sins(ang));
    ym = (s16)(50.f * coss(ang));
    m->marioObj->header.gfx.pos[0] += xm;
    m->marioObj->header.gfx.pos[2] += ym;
	
	//! this doesn't actually work according to 2021 me
	if (m->marioObj->header.gfx.animInfo.animFrame < 0) m->marioObj->header.gfx.animInfo.animFrame = 0;
	
	prevControlStickY = m->controller->stickY;
	return FALSE;
}

s32 act_squatkick(struct MarioState *m) {
	// m->actionState should be zero by default

	set_mario_animation (m, MARIO_ANIM_SQUATKICKING);

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
