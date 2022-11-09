// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Exporters/GLTFExporter.h"
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
	TArray<FString> Strings;
};

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegateGLTFExport, bool, bIsSuccessfull, FGLTFExportMessages, OutMessages);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegateOpenFile, FSelectedFiles, OutFileNames);

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

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Select File From Dialog", ToolTip = "Description.", Keywords = "select, file, dialog, windows, explorer"), Category = "File Converters|File Dialog")
	static void SelectFileFromDialog(FDelegateOpenFile DelegateFileNames, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool IsNormalizeOutputs = true);

};
