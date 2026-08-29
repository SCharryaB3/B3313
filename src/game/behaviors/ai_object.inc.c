#include "../ai_obj_helper.h"

void bhv_ai_object_main(void) {
    u32 AiObjBhvVar = 0;

    AiObjBhvVar = (o->oBehParams >> 24);

    switch (AiObjBhvVar) {
        default:
        gCurrHudType = AiObjBhvVar;
        break;

        case 0:
        break; // real Ai Obj doesnt update the HUD when BhvParam1 is 0
    }
    o->activeFlags = ACTIVE_FLAG_DEACTIVATED; //troll completed.
}

