import os, re, glob

base = r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client'
fixed = 0
for root, dirs, files in os.walk(base):
    for f in files:
        if not (f.endswith('.h') or f.endswith('.cpp')):
            continue
        path = os.path.join(root, f)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                content = fh.read()
            new = re.sub(r'(#include\s+"[^"]*/)random\.h(")', r'\1Random.h\2', content)
            new = re.sub(r'(#include\s+")random\.h(")', r'\1Random.h\2', new)
            if new != content:
                with open(path, 'w', encoding='utf-8') as fh:
                    fh.write(new)
                fixed += 1
        except Exception as e:
            pass
print(f'Fixed {fixed} files')
