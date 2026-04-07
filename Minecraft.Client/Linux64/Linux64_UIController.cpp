#ifdef _LINUX64
#include "stdafx.h"
#include "Linux64/Linux64_UIController.h"
#include "../Windows64/Iggy/gdraw/gdraw_wgl.h"

ConsoleUIController ui;

void ConsoleUIController::init(S32 w, S32 h)
{
	preInit(w, h);

	gdraw_GL_SetResourceLimits(GDRAW_GL_RESOURCE_vertexbuffer, 5000, 16 * 1024 * 1024);
	gdraw_GL_SetResourceLimits(GDRAW_GL_RESOURCE_texture, 5000, 128 * 1024 * 1024);
	gdraw_GL_SetResourceLimits(GDRAW_GL_RESOURCE_rendertarget, 10, 64 * 1024 * 1024);

	gdraw_funcs = gdraw_GL_CreateContext(w, h, 0);
	if (gdraw_funcs == nullptr)
	{
		app.DebugPrintf("[Linux64] Failed to initialise OpenGL GDraw.\n");
		app.FatalLoadError();
		return;
	}

	IggySetGDraw(gdraw_funcs);
	postInit();
}

void ConsoleUIController::shutdown()
{
	gdraw_GL_DestroyContext();
}

void ConsoleUIController::render()
{
	gdraw_GL_SetTileOrigin(0, 0, 0);
	renderScenes();
	gdraw_GL_NoMoreGDrawThisFrame();
}

void ConsoleUIController::beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion *region, CustomDrawData *customDrawRegion)
{
	gdraw_GL_BeginCustomDraw(region, customDrawRegion->mat);
}

CustomDrawData *ConsoleUIController::setupCustomDraw(UIScene *scene, IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	gdraw_GL_BeginCustomDraw(region, customDrawRegion->mat);
	setupCustomDrawGameStateAndMatrices(scene, customDrawRegion);
	return customDrawRegion;
}

CustomDrawData *ConsoleUIController::calculateCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	CustomDrawData *customDrawRegion = new CustomDrawData();
	customDrawRegion->x0 = region->x0;
	customDrawRegion->x1 = region->x1;
	customDrawRegion->y0 = region->y0;
	customDrawRegion->y1 = region->y1;

	gdraw_GL_BeginCustomDraw(region, customDrawRegion->mat);
	gdraw_GL_EndCustomDraw(region);
	return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion *region)
{
	gdraw_GL_EndCustomDraw(region);
	endCustomDrawGameStateAndMatrices();
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos)
{
	gdraw_GL_SetTileOrigin(xPos, yPos, 0);
}

GDrawTexture *ConsoleUIController::getSubstitutionTexture(int textureId)
{
	int width = 0;
	int height = 0;
	const unsigned int textureHandle = RenderManager.TextureGetHandle(textureId, &width, &height);
	if (textureHandle == 0 || width <= 0 || height <= 0)
	{
		return nullptr;
	}

	return gdraw_GL_WrappedTextureCreate(static_cast<S32>(textureHandle), width, height, false);
}

void ConsoleUIController::destroySubstitutionTexture(void *, GDrawTexture *handle)
{
	gdraw_GL_WrappedTextureDestroy(handle);
}
#endif
