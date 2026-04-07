#pragma once

#include "../Common/UI/UIController.h"

class ConsoleUIController : public UIController
{
public:
    void init(S32 w, S32 h);
    void shutdown();

    void render() override;
    void beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region, CustomDrawData *customDrawRegion) override;
    CustomDrawData *setupCustomDraw(UIScene *scene, IggyCustomDrawCallbackRegion *region) override;
    CustomDrawData *calculateCustomDraw(IggyCustomDrawCallbackRegion *region) override;
    void endCustomDraw(IggyCustomDrawCallbackRegion *region) override;

protected:
    void setTileOrigin(S32 xPos, S32 yPos) override;
    GDrawTexture *getSubstitutionTexture(int textureId) override;
    void destroySubstitutionTexture(void *destroyCallBackData, GDrawTexture *handle) override;
};

extern ConsoleUIController ui;
