import re, os, glob

roots = [
    r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client\Minecraft.World',
    r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client\Minecraft.Client\Common',
    r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client\Minecraft.Client\Linux64',
]

fixed_count = 0
for root in roots:
    for ext in ['*.h', '*.cpp']:
        for path in glob.glob(os.path.join(root, '**', ext), recursive=True):
            try:
                with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                original = content
                content = content.replace('../Minecraft.Client/', 'Minecraft.Client/')
                content = content.replace('../Minecraft.World/', 'Minecraft.World/')
                if content != original:
                    with open(path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    fixed_count += 1
            except Exception as e:
                print(f'Error {path}: {e}')

print(f'Fixed {fixed_count} files')
