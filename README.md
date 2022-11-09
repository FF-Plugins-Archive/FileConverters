# FileConverters
 
Target Platform: Windows
Unreal Version: Unreal Engine 5 and up (feel free to fork and convert it to UE4)

Features:
- Runtime GLTF static mesh exporter (it works on packaged projects, too)
- Runtime "File selection Dialog"
  - Multiple selection with string array
  - Async (it doesn't block game thread even if you open a dialog)
  - Extension filtering
  - Start path selection
  - You can change dialog title
  - You can change "Ok" button title
  - WIP = Folder selection
- Some other functions for in-house usages.
