import os

kwang_global = r"d:\Unreal Projects\LedgeOut\Content\ParagonKwang\Characters\Global"
main_global = r"d:\Unreal Projects\LedgeOut\Content\Characters\Global"

kwang_files = []
for root, dirs, files in os.walk(kwang_global):
    for f in files:
        rel = os.path.relpath(os.path.join(root, f), kwang_global)
        kwang_files.append(rel)

missing_in_main = []
present_in_main = []

for rel in kwang_files:
    target = os.path.join(main_global, rel)
    if not os.path.exists(target):
        missing_in_main.append(rel)
    else:
        present_in_main.append(rel)

print(f"Total files in ParagonKwang/Characters/Global: {len(kwang_files)}")
print(f"Already existing in Content/Characters/Global: {len(present_in_main)}")
print(f"Missing in Content/Characters/Global: {len(missing_in_main)}")

if missing_in_main:
    print("\nFiles missing in main Global directory:")
    for m in missing_in_main:
        print("  -", m)
