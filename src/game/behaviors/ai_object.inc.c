#include "../personalization_helpers.h"

void bhv_ai_object_main(void) {
    u32 AiObjBhvVar = 0;

    AiObjBhvVar = (o->oBehParams >> 24);

    switch (AiObjBhvVar) {
        default:
        gCurrHudType = AiObjBhvVar;
        #ifdef ISVPRINT
        osSyncPrintf("[AI OBJECT] Current Value is %02d\n", gCurrHudType);
        #endif
        break;

        case 0:
        #ifdef ISVPRINT
        osSyncPrintf("[AI OBJECT] CASE 0\n");
        #endif
        break; // real Ai Obj doesnt update the HUD when BhvParam1 is 0
    }

    osSyncPrintf("[AI OBJECT] PASTA LA VISTA\n");
    o->activeFlags = ACTIVE_FLAG_DEACTIVATED; //troll completed.
}

