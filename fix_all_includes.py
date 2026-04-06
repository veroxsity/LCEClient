import os, re, glob

base = r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client'

fixed_files = 0
for root, dirs, files in os.walk(base):
    # Skip build dirs, .git, etc.
    dirs[:] = [d for d in dirs if d not in ('build', '.git', 'x64', 'node_modules')]
    for f in files:
        if not (f.endswith('.h') or f.endswith('.cpp') or f.endswith('.c')):
            continue
        path = os.path.join(root, f)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                content = fh.read()
            original = content
            # Fix backslash separators in #include paths
            content = re.sub(
                r'(#include\s+")([^"]+)(")',
                lambda m: m.group(1) + m.group(2).replace('\\', '/') + m.group(3),
                content
            )
            # Fix ../ prefixes that now resolve via repo root
            content = content.replace('../Minecraft.Client/', 'Minecraft.Client/')
            content = content.replace('../Minecraft.World/', 'Minecraft.World/')
            content = content.replace('../../Minecraft.Client/', 'Minecraft.Client/')
            content = content.replace('../../Minecraft.World/', 'Minecraft.World/')
            if content != original:
                with open(path, 'w', encoding='utf-8') as fh:
                    fh.write(content)
                fixed_files += 1
        except Exception as e:
            pass

print(f'Fixed {fixed_files} files')
