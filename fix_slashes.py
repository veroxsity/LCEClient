import re, os, glob

# Fix backslash includes in all .h and .cpp files in Minecraft.World and Minecraft.Client/Common
roots = [
    r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client\Minecraft.World',
]

fixed_count = 0
for root in roots:
    for ext in ['*.h', '*.cpp']:
        for path in glob.glob(os.path.join(root, ext)):
            with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            original = content
            content = re.sub(
                r'(#include\s+")([^"]+)(")',
                lambda m: m.group(1) + m.group(2).replace('\\', '/') + m.group(3),
                content
            )
            if content != original:
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(content)
                fixed_count += 1

print(f'Fixed {fixed_count} files')
