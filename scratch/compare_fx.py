import os

kwang_fx = r"d:\Unreal Projects\LedgeOut\Content\ParagonKwang\FX"
main_global = r"d:\Unreal Projects\LedgeOut\Content\Characters\Global"
kwang_dest = r"d:\Unreal Projects\LedgeOut\Content\Characters\Kwang"

fx_files = []
for root, dirs, files in os.walk(kwang_fx):
    for f in files:
        rel = os.path.relpath(os.path.join(root, f), kwang_fx)
        fx_files.append(rel)

print(f"Total files in ParagonKwang/FX: {len(fx_files)}")
