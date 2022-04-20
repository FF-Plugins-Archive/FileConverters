// Copyright Epic Games, Inc. All Rights Reserved.

#include "FileConvertersBPLibrary.h"
#include "FileConverters.h"

// UE Includes.
#include "Kismet/KismetStringLibrary.h"

UFileConvertersBPLibrary::UFileConvertersBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UFileConvertersBPLibrary::LoadAdditionalAssetString(const FString AssetRelativePath, FString& OutAssetString)
{
    const FString AssetPathCoocked = FPaths::ProjectContentDir() + AssetRelativePath;
    FFileHelper::LoadFileToString(OutAssetString, *AssetPathCoocked);
}

void UFileConvertersBPLibrary::LoadAdditionalAssetBytes(const FString AssetRelativePath, TArray<uint8>& OutAssetBytes)
{
    const FString AssetPathCoocked = FPaths::ProjectContentDir() + AssetRelativePath;
    FFileHelper::LoadFileToArray(OutAssetBytes, *AssetPathCoocked);
}

FString UFileConvertersBPLibrary::HelperIFCConverter(const FString IFC_Converter_Path, const FString IFC_EXE_Name, const FString IFC_Path)
{
    FString Clean_IFC_Path = FPaths::GetBaseFilename(IFC_Path, false);
    return TEXT("cd /d ") + IFC_Converter_Path + TEXT(" & ") + IFC_EXE_Name + TEXT(" --use-element-hierarchy --generate-uvs --center-model --center-model-geometry ") + IFC_Path + TEXT(" ") + Clean_IFC_Path + TEXT(".dae");
}

FString UFileConvertersBPLibrary::CreatePDFViewer(const FString In_HTML_Content, const FString In_PDF_Path, TArray<uint8> In_PDF_Bytes, const FString DummyText, bool bUseBytes)
{
    FString PDF_Base64;
    
    if (bUseBytes == false)
    {
        // Load PFD and Convert it to Base64.
        TArray<uint8> PDF_Bytes;
        FFileHelper::LoadFileToArray(PDF_Bytes, *In_PDF_Path);
        PDF_Base64 = FBase64::Encode(PDF_Bytes);
    }

    else
    {
        PDF_Base64 = FBase64::Encode(In_PDF_Bytes);
    }

    // Replace dummy text in HTML content to Base64 PDF.
    FString HTML_Content = In_HTML_Content;
    HTML_Content.ReplaceInline(*DummyText, *PDF_Base64, ESearchCase::CaseSensitive);

    return HTML_Content;
}