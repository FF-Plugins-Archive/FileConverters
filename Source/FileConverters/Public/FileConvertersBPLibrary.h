// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Exporters/GLTFExporter.h"					// FDelegateGLTFExport -> Export Messages.
#include "FileConvertersBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

USTRUCT(BlueprintType)
struct FSelectedFiles
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	bool IsSuccessfull = false;

	UPROPERTY(BlueprintReadOnly)
	bool IsFolder = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Strings;
};

USTRUCT(BlueprintType)
struct FFolderContent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Path = "";

	UPROPERTY(BlueprintReadOnly)
	FString Name = "";

	UPROPERTY(BlueprintReadOnly)
	bool bIsFile = false;
};

USTRUCT(BlueprintType)
struct FContentArrayContainer
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	TArray<FFolderContent> OutContents;
};

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegateGLTFExport, bool, bIsSuccessfull, FGLTFExportMessages, OutMessages);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegateOpenFile, FSelectedFiles, OutFileNames);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegateSaveFile, bool, bIsSaveSuccessful, FString, OutFileName);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegateSearch, bool, bIsSearchSuccessful, FString, ErrorCode, FContentArrayContainer, Out);

UCLASS()
class UFileConvertersBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Load Additional Asset String", ToolTip = "Write relative file path after Content.", Keywords = "load, asset, additional, non-coocked, string"), Category = "File Converters|Load")
	static void LoadAdditionalAssetString(const FString AssetRelativePath, FString& OutAssetString);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Load Additional Asset Bytes", ToolTip = "Write relative file path after Content.", Keywords = "load, asset, additional, non-coocked, bytes"), Category = "File Converters|Load")
	static void LoadAdditionalAssetBytes(const FString AssetRelativePath, TArray<uint8>& OutAssetBytes);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Helper IFC Converter", Keywords = "cmd, helper, converter, ifc"), Category = "File Converters|CMD")
	static FString HelperIFCConverter(const FString IFC_Converter_Path, const FString IFC_EXE_Name, const FString IFC_Path);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create PDF Viewer", ToolTip = "Default DummyText is ff_base64. If you use path for PDFs, you should attach an MakeArray node to In PDF Bytes. Because that input requires an attachment and it is about unreal blueprint limitations.", Keywords = "create, view, show, pdf, pdfjs, viewer"), Category = "File Converters|HTML")
	static FString CreatePDFViewer(const FString In_HTML_Content, const FString In_PDF_Path, TArray<uint8> In_PDF_Bytes, const FString DummyText, bool bUseBytes);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Export Level As GLTF", ToolTip = "Description.", Keywords = "level, export, gltf, glb"), Category = "File Converters|GLTF")
	static void ExportLevelGLTF(bool bEnableQuantization, bool bResetLocation, bool bResetRotation, bool bResetScale, const FString ExportPath, TSet<AActor*> TargetActors, FDelegateGLTFExport DelegateGLTFExport);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Select File From Dialog", ToolTip = "If you enable \"Allow Folder Selection\", extension filtering will be disabled. \nExtension filtering uses a String to String MAP variable. \nKey is description and value is extension's itself. You need to write like this without quotes \"*.extension\". \nIf one extension group has multiple extensions, you need to use \";\" after each one.", Keywords = "select, file, folder, dialog, windows, explorer"), Category = "File Converters|File Dialog")
	static void SelectFileFromDialog(FDelegateOpenFile DelegateFileNames, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs = true, bool bAllowFolderSelection = false);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save File with Dialog", ToolTip = "Each extension group must have only one extension. \nIf that group has multiple variation, you should define one by one all of them if you need them. \nAlso you need to write them as \"*.extension\".", Keywords = "save, file, dialog, windows, explorer"), Category = "File Converters|File Dialog")
	static void SaveFileDialog(FDelegateSaveFile DelegateSaveFile, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Folder Contents", ToolTip = "Description.", Keywords = "explorer, load, file, folder, content"), Category = "File Converters|File Dialog")
	static bool GetFolderContents(TArray<FFolderContent>& OutContents, FString& ErrorCode, FString InPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Search In Folder", ToolTip = "Description.", Keywords = "explorer, load, file, folder, content"), Category = "File Converters|File Dialog")
	static void SearchInFolder(FDelegateSearch DelegateSearch, FString InPath, FString InSearch, bool bSearchExact);

};
