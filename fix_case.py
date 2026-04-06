import os, re, glob

base = r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client'

# The complete map of wrong -> correct
FIXES = {
    '4J_XTMS.h': '4J_xtms.h',
    'App_defines.h': 'App_Defines.h',
    'AtgXmlParser.h': 'ATGXmlParser.h',
    'Enderdragon.h': 'EnderDragon.h',
    'Glowstonetile.h': 'GlowstoneTile.h',
    'MultiplayerLevel.h': 'MultiPlayerLevel.h',
    'MultiplayerLocalPlayer.h': 'MultiPlayerLocalPlayer.h',
    'Potion_macros.h': 'Potion_Macros.h',
    'PreloginPacket.h': 'PreLoginPacket.h',
    'Random.h': 'random.h',
    'RedstoneItem.h': 'RedStoneItem.h',
    'SnowBallItem.h': 'SnowballItem.h',
    'SnowmanModel.h': 'SnowManModel.h',
    'SnowmanRenderer.h': 'SnowManRenderer.h',
    'StdAfx.h': 'stdafx.h',
    'Stubs_SPU.h': 'stubs_SPU.h',
    'WaterlilyTile_SPU.h': 'WaterLilyTile_SPU.h',
    'XUI_Ctrl_PassThroughList.h': 'XUI_Ctrl_PassthroughList.h',
    'biome.h': 'Biome.h',
    'class.h': 'Class.h',
    'dimension.h': 'Dimension.h',
    'entityRenderDispatcher.h': 'EntityRenderDispatcher.h',
    'facing.h': 'Facing.h',
    'item.h': 'Item.h',
    'leveldata.h': 'LevelData.h',
    'mapitem.h': 'MapItem.h',
    'mob.h': 'Mob.h',
    'mth.h': 'Mth.h',
    'net.minecraft.world.Item.alchemy.h': 'net.minecraft.world.item.alchemy.h',
    'net.minecraft.world.Item.h': 'net.minecraft.world.item.h',
    'net.minecraft.world.entity.Item.h': 'net.minecraft.world.entity.item.h',
    'path.h': 'Path.h',
    'pos.h': 'Pos.h',
    'resource.h': 'Resource.h',
    'rrcore.h': 'rrCore.h',
    'tile.h': 'Tile.h',
    'tileRenderer.h': 'TileRenderer.h',
    'treetile.h': 'TreeTile.h',
    'vertex.h': 'Vertex.h',
    'woodtile.h': 'WoodTile.h',
    'xuiscene_Death.h': 'xuiscene_death.h',
    'xuiscene_DebugTips.h': 'xuiscene_debugtips.h',
    'xuiscene_Pause.h': 'xuiscene_pause.h',
    'xuiscene_Reinstall.h': 'xuiscene_reinstall.h',
    'xuiscene_Win.h': 'xuiscene_win.h',
    'xuiscene_settings_all.h': 'xuiscene_settings_All.h',
    'xuiscene_settings_audio.h': 'xuiscene_settings_Audio.h',
    'xuiscene_settings_control.h': 'xuiscene_settings_Control.h',
    'xuiscene_settings_graphics.h': 'xuiscene_settings_Graphics.h',
}

def fix_include_line(line):
    m = re.search(r'(#include\s+"[^"]*/)([^"/]+)(")', line)
    if m:
        basename = m.group(2)
        if basename in FIXES:
            return line.replace(basename, FIXES[basename])
    # Also handle includes without path prefix
    m2 = re.search(r'(#include\s+")([^"/]+)(")', line)
    if m2:
        basename = m2.group(2)
        if basename in FIXES:
            return line.replace(basename, FIXES[basename])
    return line

fixed_files = 0
for root, dirs, files in os.walk(base):
    for f in files:
        if not (f.endswith('.h') or f.endswith('.cpp')):
            continue
        path = os.path.join(root, f)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                lines = fh.readlines()
            new_lines = [fix_include_line(l) for l in lines]
            if new_lines != lines:
                with open(path, 'w', encoding='utf-8') as fh:
                    fh.writelines(new_lines)
                fixed_files += 1
        except Exception as e:
            print(f'Error {path}: {e}')

print(f'Fixed {fixed_files} files')
