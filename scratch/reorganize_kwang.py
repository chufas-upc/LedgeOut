import unreal

def reorganize_kwang():
    print("=" * 60)
    print("STARTING PARAGON KWANG AUTOMATED REORGANIZATION")
    print("=" * 60)
    
    asset_lib = unreal.EditorAssetLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # 1. Handle ParagonKwang/Characters/Global -> Characters/Global
    src_global = "/Game/ParagonKwang/Characters/Global"
    tgt_global = "/Game/Characters/Global"
    
    if asset_lib.does_directory_exist(src_global):
        assets = asset_lib.list_assets(src_global, recursive=True, include_folder=False)
        print(f"Found {len(assets)} assets in {src_global}")
        
        consolidated = 0
        moved = 0
        errors = 0
        
        for src_path in assets:
            rel_path = src_path[len(src_global):]
            tgt_path = tgt_global + rel_path
            
            if asset_lib.does_asset_exist(tgt_path):
                tgt_obj = asset_lib.load_asset(tgt_path)
                src_obj = asset_lib.load_asset(src_path)
                if tgt_obj and src_obj:
                    print(f"[CONSOLIDATE] Replacing references: {src_path} -> {tgt_path}")
                    if asset_lib.consolidate_assets(tgt_obj, [src_obj]):
                        consolidated += 1
                    else:
                        print(f"[ERROR] Failed to consolidate {src_path}")
                        errors += 1
                else:
                    print(f"[ERROR] Could not load assets: {src_path} or {tgt_path}")
                    errors += 1
            else:
                print(f"[MOVE] Moving unique asset: {src_path} -> {tgt_path}")
                if asset_lib.rename_asset(src_path, tgt_path):
                    moved += 1
                else:
                    print(f"[ERROR] Failed to move {src_path}")
                    errors += 1
                    
        print(f"Global Assets Summary: Consolidated {consolidated}, Moved {moved}, Errors {errors}")

    # 2. Handle ParagonKwang/FX -> Characters/Kwang/FX
    src_fx = "/Game/ParagonKwang/FX"
    tgt_fx = "/Game/Characters/Kwang/FX"
    
    if asset_lib.does_directory_exist(src_fx):
        fx_assets = asset_lib.list_assets(src_fx, recursive=True, include_folder=False)
        print(f"Found {len(fx_assets)} assets in {src_fx}")
        
        consolidated = 0
        moved = 0
        
        for src_path in fx_assets:
            rel_path = src_path[len(src_fx):]
            tgt_path = tgt_fx + rel_path
            
            if asset_lib.does_asset_exist(tgt_path):
                tgt_obj = asset_lib.load_asset(tgt_path)
                src_obj = asset_lib.load_asset(src_path)
                if tgt_obj and src_obj:
                    asset_lib.consolidate_assets(tgt_obj, [src_obj])
                    consolidated += 1
            else:
                asset_lib.rename_asset(src_path, tgt_path)
                moved += 1
                
        print(f"FX Assets Summary: Consolidated {consolidated}, Moved {moved}")

    # 3. Fix Up Redirectors
    print("Gathering redirectors to fix up...")
    all_assets = asset_lib.list_assets("/Game", recursive=True, include_folder=False)
    redirectors = []
    for a_path in all_assets:
        a_data = unreal.AssetRegistryHelpers.get_asset_registry().get_asset_by_object_path(a_path)
        if a_data and a_data.is_redirector():
            redir_obj = asset_lib.load_asset(a_path)
            if redir_obj:
                redirectors.append(redir_obj)
                
    if redirectors:
        print(f"Fixing up {len(redirectors)} redirectors...")
        asset_tools.fix_up_referencers(redirectors)
    else:
        print("No redirectors found to fix up.")

    # 4. Save modified assets
    print("Saving directories...")
    asset_lib.save_directory("/Game/Characters", only_if_is_dirty=False, recursive=True)
    
    # 5. Clean up ParagonKwang directory if empty
    if asset_lib.does_directory_exist("/Game/ParagonKwang"):
        print("Deleting empty directory /Game/ParagonKwang...")
        asset_lib.delete_directory("/Game/ParagonKwang")
        
    print("=" * 60)
    print("AUTOMATED REORGANIZATION COMPLETED SUCCESSFULLY!")
    print("=" * 60)

# Automatically run when script is executed
reorganize_kwang()
