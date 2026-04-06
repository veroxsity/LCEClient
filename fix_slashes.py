import re, os, glob

roots = [
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
                content = re.sub(
                    r'(#include\s+")([^"]+)(")',
                    lambda m: m.group(1) + m.group(2).replace('\\', '/') + m.group(3),
                    content
                )
                if content != original:
                    with open(path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    fixed_count += 1
                    print(f'  Fixed: {os.path.basename(path)}')
            except Exception as e:
                print(f'  Error {path}: {e}')

print(f'Total: {fixed_count} files fixed')
