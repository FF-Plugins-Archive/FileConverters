// Copyright Epic Games, Inc. All Rights Reserved.

#include "FileConvertersBPLibrary.h"
#include "FileConverters.h"

// UE Includes.
#include "Kismet/KismetStringLibrary.h"
#include "Builders/GLTFBuilder.h"
#include "UserData/GLTFMaterialUserData.h"

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

void UFileConvertersBPLibrary::ExportLevelGLTF(bool bEnableQuantization, bool bResetLocation, bool bResetRotation, bool bResetScale, const FString ExportPath, TSet<AActor*> TargetActors, FDelegateGLTFExport DelegateGLTFExport)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateGLTFExport, bEnableQuantization, bResetLocation, bResetRotation, bResetScale, ExportPath, TargetActors]()
        {
            TArray<FVector> Array_Locations;
            TArray<FRotator> Array_Rotations;
            TArray<FVector> Array_Scales;

            for (int32 ActorIndex = 0; ActorIndex < TargetActors.Num(); ActorIndex++)
            {
                if (bResetLocation == true)
                {
                    Array_Locations.Add(TargetActors.Array()[ActorIndex]->GetRootComponent()->GetComponentLocation());
                    TargetActors.Array()[ActorIndex]->GetRootComponent()->SetWorldLocation(FVector(0.0f), false, nullptr, ETeleportType::None);
                }

                if (bResetRotation == true)
                {
                    Array_Rotations.Add(TargetActors.Array()[ActorIndex]->GetRootComponent()->GetComponentRotation());
                    TargetActors.Array()[ActorIndex]->GetRootComponent()->SetWorldRotation(FQuat(0.0f), false, nullptr, ETeleportType::None);
                }

                if (bResetScale == true)
                {
                    Array_Scales.Add(TargetActors.Array()[ActorIndex]->GetRootComponent()->GetComponentScale());
                    TargetActors.Array()[ActorIndex]->GetRootComponent()->SetWorldScale3D(FVector(1.0f));
                }
            }

            AsyncTask(ENamedThreads::GameThread, [DelegateGLTFExport, bEnableQuantization, bResetLocation, bResetRotation, bResetScale, Array_Locations, Array_Rotations, Array_Scales, ExportPath, TargetActors]()
                {
                    UGLTFExportOptions* ExportOptions = NewObject<UGLTFExportOptions>();
                    ExportOptions->ResetToDefault();
                    ExportOptions->bExportProxyMaterials = true;
                    ExportOptions->bExportVertexColors = true;
                    ExportOptions->bUseMeshQuantization = bEnableQuantization;

                    FGLTFExportMessages ExportMessages;
                    bool bIsExportSuccessful = UGLTFExporter::ExportToGLTF(GEngine->GetCurrentPlayWorld(), ExportPath, ExportOptions, TargetActors, ExportMessages);

                    for (int32 ActorIndex = 0; ActorIndex < TargetActors.Num(); ActorIndex++)
                    {
                        if (bResetLocation == true)
                        {
                            TargetActors.Array()[ActorIndex]->GetRootComponent()->SetWorldLocation(Array_Locations[ActorIndex], false, nullptr, ETeleportType::None);
                        }

                        if (bResetRotation == true)
                        {
                            TargetActors.Array()[ActorIndex]->GetRootComponent()->SetWorldRotation(Array_Rotations[ActorIndex], false, nullptr, ETeleportType::None);
                        }

                        if (bResetScale == true)
                        {
                            TargetActors.Array()[ActorIndex]->GetRootComponent()->SetWorldScale3D(Array_Scales[ActorIndex]);
                        }
                    }

                    DelegateGLTFExport.ExecuteIfBound(bIsExportSuccessful, ExportMessages);
                }
            );
        }
    );
}