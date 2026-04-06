import os, re, glob

# Build a map of actual filenames (case-sensitive) in Minecraft.Client and Minecraft.World
base = r'C:\Users\Dan\Documents\Programming\.MinecraftLegacyEdition\client'
actual_files = {}

for root, dirs, files in os.walk(base):
    for f in files:
        lower = f.lower()
        if lower not in actual_files:
            actual_files[lower] = f  # store actual cased name

# Now scan all include directives and find case mismatches
mismatches = set()
for root, dirs, files in os.walk(base):
    for f in files:
        if not (f.endswith('.h') or f.endswith('.cpp')):
            continue
        path = os.path.join(root, f)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                for line in fh:
                    m = re.search(r'#include\s+"([^"]+)"', line)
                    if not m:
                        continue
                    inc = m.group(1)
                    basename = inc.split('/')[-1].split('\\')[-1]
                    lower = basename.lower()
                    if lower in actual_files and actual_files[lower] != basename:
                        mismatches.add((basename, actual_files[lower]))
        except:
            pass

for wrong, correct in sorted(mismatches):
    print(f'  WRONG: {wrong}  ->  CORRECT: {correct}')
print(f'\nTotal mismatches: {len(mismatches)}')
