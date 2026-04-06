#ifdef _LINUX64
#include "stdafx.h"
#include "Linux64/Linux64_UIController.h"

ConsoleUIController ui;

void ConsoleUIController::init(S32 w, S32 h)
{
    preInit(w, h);
    postInit();
}

void ConsoleUIController::shutdown()
{
}

void ConsoleUIController::render()
{
    renderScenes();
}

void ConsoleUIController::beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *, CustomDrawData *)
{
}

CustomDrawData *ConsoleUIController::setupCustomDraw(UIScene *, IggyCustomDrawCallbackRegion *region)
{
    CustomDrawData *customDrawRegion = new CustomDrawData();
    customDrawRegion->x0 = region->x0;
    customDrawRegion->x1 = region->x1;
    customDrawRegion->y0 = region->y0;
    customDrawRegion->y1 = region->y1;
    for (float &value : customDrawRegion->mat)
    {
        value = 0.0f;
    }
    customDrawRegion->mat[0] = 1.0f;
    customDrawRegion->mat[5] = 1.0f;
    customDrawRegion->mat[10] = 1.0f;
    customDrawRegion->mat[15] = 1.0f;
    return customDrawRegion;
}

CustomDrawData *ConsoleUIController::calculateCustomDraw(IggyCustomDrawCallbackRegion *region)
{
    return setupCustomDraw(nullptr, region);
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion *)
{
}

void ConsoleUIController::setTileOrigin(S32, S32)
{
}
#endif
