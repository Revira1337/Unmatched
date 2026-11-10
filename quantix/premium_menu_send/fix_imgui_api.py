import os
import re

# Files to fix
files_to_fix = [
    r"c:\Users\hardo\Desktop\Quantix\premium_menu_send\framework\widgets\checkbox_color.cpp",
    r"c:\Users\hardo\Desktop\Quantix\premium_menu_send\framework\widgets\input.cpp",
    r"c:\Users\hardo\Desktop\Quantix\premium_menu_send\framework\widgets\combo.cpp",
    r"c:\Users\hardo\Desktop\Quantix\premium_menu_send\framework\widgets\window.cpp",
]

# Replacements
replacements = [
    (r'\.CurLenA\b', '.TextLen'),
    (r'\.BufCapacityA\b', '.BufCapacity'),
    (r'\.InitialTextA\b', '.TextToRevertTo.Data'),
    (r'\.ReloadUserBuf\b', '.WantReloadUserBuf'),
    (r'\.InFlags\b', '.ItemFlags'),
    (r'\.Flags\b', '.Flags'),  # ImGuiNextWindowData still has Flags but we can't replace all
    (r'ImGuiButtonFlags_NoKeyModifiers', 'ImGuiButtonFlags_NoKeyModsAllowed'),
    (r'ImGuiButtonFlags_Repeat', 'ImGuiItemFlags_ButtonRepeat'),
    (r'ImGuiButtonFlags_DontClosePopups', 'ImGuiButtonFlags_None'),
    (r'ImGuiInputTextFlags_NoMarkEdited', 'ImGuiItemFlags_NoMarkEdited'),
]

for filepath in files_to_fix:
    if not os.path.exists(filepath):
        print(f"Skipping {filepath} - file not found")
        continue
    
    print(f"Processing {filepath}...")
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    
    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)
    
    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"  Fixed {filepath}")
    else:
        print(f"  No changes needed in {filepath}")

print("\nDone!")
