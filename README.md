# FileConverters
 
Target Platform: Windows (both editor runtime and packaged runtime)
Unreal Version: Unreal Engine 5 and up (feel free to fork and convert it to UE4)

Features:
- Runtime GLTF static mesh exporter (it works on packaged projects, too)
- Runtime file selection dialog
	- Multiple selection with string array
	- Async (it doesn't block game thread even if you open a dialog)
	- Extension filtering
	- Start path selection
	- You can change dialog title
	- You can change "Ok" button title
	- Runtime folder selection
- Runtime Save Dialog
	- Async (it doesn't block game thread even if you open a dialog) 
	- Save extension selection
	- Start path selection
	- You can change dialog title
	- You can change "Ok" button title
- Some other functions (CMD CLI helpers) for in-house usages.

You can find sample nodes and their usages in Plugins/FileConverters Content/BPs/BP_Template_FileConverters
You need to open Plugins folder from "Content Browser" settings at the bottom of editor.

*USER THIS IS NOT A RUNTIME IMPORT LIBRARY ! THIS ONLY GIVES SELECTED FILE'S OR FOLDER'S PATH !
*WE HAVE NECESSARY LIBRARIES TO IMPORT CAD FILES, SO WE USED THE IN OUR SAMPLE.
*YOU NEED TO CREATE YOUR OWN MECHANICS TO USE (IMPORT) THAT FILES !

File Selection Sample Blueprint
![FileSelection](https://user-images.githubusercontent.com/10528447/201652963-b730b091-3be8-47e6-a534-33eb607d8880.jpg)

Save Dialog Sample Blueprint
![SaveDialog](https://user-images.githubusercontent.com/10528447/201652972-f93f7351-f665-4f24-ab5a-7e75b249dd22.jpg)
