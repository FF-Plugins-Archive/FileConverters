// Copyright Epic Games, Inc. All Rights Reserved.

#include "FileConvertersBPLibrary.h"
#include "FileConverters.h"

// UE Includes.
#include "Builders/GLTFBuilder.h"
#include "UserData/GLTFMaterialUserData.h"
#include "GenericPlatform/GenericPlatformMisc.h"
//#include "Kismet/KismetStringLibrary.h"

// Windows Includes.
#define WIN32_LEAN_AND_MEAN
#include "shobjidl_core.h"

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

void UFileConvertersBPLibrary::SelectFileFromDialog(FDelegateOpenFile DelegateFileNames, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs, bool bAllowFolderSelection)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateFileNames, InDialogName, InOkLabel, InDefaultPath, InExtensions, DefaultExtensionIndex, bIsNormalizeOutputs, bAllowFolderSelection]()
        {
            IFileOpenDialog* FileOpenDialog;
            HRESULT FileDialogInstance = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&FileOpenDialog));

            IShellItemArray* ShellItems;
            TArray<FString> Array_FilePaths;

            // If dialog instance successfully created.
            if (SUCCEEDED(FileDialogInstance))
            {
                // https://stackoverflow.com/questions/70174174/c-com-comdlg-filterspec-array-overrun
                int32 ExtensionCount = InExtensions.Num();
                COMDLG_FILTERSPEC* ExtensionArray = new COMDLG_FILTERSPEC[ExtensionCount];
                COMDLG_FILTERSPEC* EachExtension = ExtensionArray;

                TArray<FString> ExtensionKeys;
                InExtensions.GetKeys(ExtensionKeys);

                TArray<FString> ExtensionValues;
                InExtensions.GenerateValueArray(ExtensionValues);
                
                for (int32 ExtensionIndex = 0; ExtensionIndex < ExtensionCount; ExtensionIndex++)
                {
                    EachExtension->pszName = *ExtensionKeys[ExtensionIndex];
                    EachExtension->pszSpec = *ExtensionValues[ExtensionIndex];
                    ++EachExtension;
                }

                FileOpenDialog->SetFileTypes(ExtensionCount, ExtensionArray);
                
                // Starts from 1
                FileOpenDialog->SetFileTypeIndex(DefaultExtensionIndex+1);
                
                DWORD dwOptions;
                FileOpenDialog->GetOptions(&dwOptions);
                
                if (bAllowFolderSelection == true)
                {   
                    // https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions
                    FileOpenDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_OKBUTTONNEEDSINTERACTION);
                }

                else
                {
                    // https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions
                    FileOpenDialog->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_OKBUTTONNEEDSINTERACTION);
                }
               
                if (InDialogName.IsEmpty() != true)
                {
                    FileOpenDialog->SetTitle(*InDialogName);
                }

                if (InOkLabel.IsEmpty() != true)
                {
                    FileOpenDialog->SetOkButtonLabel(*InOkLabel);
                }

                if (InDefaultPath.IsEmpty() != true)
                {
                    FString DefaultPathString = InDefaultPath;

                    FPaths::MakePlatformFilename(DefaultPathString);

                    IShellItem* DefaultFolder = NULL;
                    HRESULT DefaultPathResult = SHCreateItemFromParsingName(*DefaultPathString, nullptr, IID_PPV_ARGS(&DefaultFolder));
                    
                    if (SUCCEEDED(DefaultPathResult))
                    {
                        FileOpenDialog->SetFolder(DefaultFolder);
                        DefaultFolder->Release();
                    }
                }
     
                HWND WindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());
                FileDialogInstance = FileOpenDialog->Show(WindowHandle);
                
                // If dialog successfully showed up.
                if (SUCCEEDED(FileDialogInstance))
                {
                    FileDialogInstance = FileOpenDialog->GetResults(&ShellItems);

                    // Is results got.
                    if (SUCCEEDED(FileDialogInstance))
                    {
                        DWORD ItemCount;
                        ShellItems->GetCount(&ItemCount);

                        for (DWORD ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
                        {
                            IShellItem* EachItem;
                            ShellItems->GetItemAt(ItemIndex, &EachItem);

                            PWSTR EachFilePathSTR = NULL;
                            EachItem->GetDisplayName(SIGDN_FILESYSPATH, &EachFilePathSTR);
                            
                            FString EachFilePath = EachFilePathSTR;
                            
                            if (bIsNormalizeOutputs == true)
                            {
                                FPaths::NormalizeFilename(EachFilePath);
                            }

                            Array_FilePaths.Add(EachFilePath);

                            EachItem->Release();
                        }

                        ShellItems->Release();
                        FileOpenDialog->Release();
                        CoUninitialize();

                        AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, Array_FilePaths, bAllowFolderSelection]()
                            {
                                if (Array_FilePaths.IsEmpty() == false)
                                {
                                    FSelectedFiles SelectedFiles;
                                    SelectedFiles.IsSuccessfull = true;
                                    SelectedFiles.IsFolder = bAllowFolderSelection;
                                    SelectedFiles.Strings = Array_FilePaths;

                                    DelegateFileNames.ExecuteIfBound(SelectedFiles);
                                }

                                else
                                {
                                    FSelectedFiles SelectedFiles;
                                    SelectedFiles.IsSuccessfull = false;
                                    SelectedFiles.IsFolder = bAllowFolderSelection;

                                    DelegateFileNames.ExecuteIfBound(SelectedFiles);
                                }
                            }
                        );

                    }

                    // Function couldn't get results.
                    else
                    {
                        AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, ShellItems, FileOpenDialog, bAllowFolderSelection]()
                            {
                                FileOpenDialog->Release();
                                CoUninitialize();
                                
                                FSelectedFiles SelectedFiles;
                                SelectedFiles.IsSuccessfull = false;
                                SelectedFiles.IsFolder = bAllowFolderSelection;

                                DelegateFileNames.ExecuteIfBound(SelectedFiles);
                            }
                        );
                    }
                }
                
                // Dialog didn't show up.
                else
                {
                    AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, FileOpenDialog, ShellItems, bAllowFolderSelection]()
                        {
                            FileOpenDialog->Release();
                            CoUninitialize();
                            
                            FSelectedFiles SelectedFiles;
                            SelectedFiles.IsSuccessfull = false;
                            SelectedFiles.IsFolder = bAllowFolderSelection;

                            DelegateFileNames.ExecuteIfBound(SelectedFiles);
                        }
                    );
                }
            }

            // Function couldn't create dialog.
            else
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, FileOpenDialog, ShellItems, bAllowFolderSelection]()
                    {
                        FileOpenDialog->Release();
                        CoUninitialize();
                        
                        FSelectedFiles SelectedFiles;
                        SelectedFiles.IsSuccessfull = false;
                        SelectedFiles.IsFolder = bAllowFolderSelection;

                        DelegateFileNames.ExecuteIfBound(SelectedFiles);
                    }
                );
            }
        }
    );
}

void UFileConvertersBPLibrary::SaveFileDialog(FDelegateSaveFile DelegateSaveFile, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateSaveFile, InDialogName, InOkLabel, InDefaultPath, InExtensions, DefaultExtensionIndex, bIsNormalizeOutputs]()
        {
            IFileSaveDialog* SaveFileDialog;
            HRESULT SaveDialogInstance = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&SaveFileDialog));

            IShellItem* ShellItem;

            // If dialog instance successfully created.
            if (SUCCEEDED(SaveDialogInstance))
            {
                // https://stackoverflow.com/questions/70174174/c-com-comdlg-filterspec-array-overrun
                int32 ExtensionCount = InExtensions.Num();
                COMDLG_FILTERSPEC* ExtensionArray = new COMDLG_FILTERSPEC[ExtensionCount];
                COMDLG_FILTERSPEC* EachExtension = ExtensionArray;

                TArray<FString> ExtensionKeys;
                InExtensions.GetKeys(ExtensionKeys);

                TArray<FString> ExtensionValues;
                InExtensions.GenerateValueArray(ExtensionValues);

                for (int32 ExtensionIndex = 0; ExtensionIndex < ExtensionCount; ExtensionIndex++)
                {
                    EachExtension->pszName = *ExtensionKeys[ExtensionIndex];
                    EachExtension->pszSpec = *ExtensionValues[ExtensionIndex];
                    ++EachExtension;
                }
               
                SaveFileDialog->SetFileTypes(ExtensionCount, ExtensionArray);

                // Starts from 1
                SaveFileDialog->SetFileTypeIndex(DefaultExtensionIndex + 1);

                DWORD dwOptions;
                SaveFileDialog->GetOptions(&dwOptions);

                if (InDialogName.IsEmpty() != true)
                {
                    SaveFileDialog->SetTitle(*InDialogName);
                }

                if (InOkLabel.IsEmpty() != true)
                {
                    SaveFileDialog->SetOkButtonLabel(*InOkLabel);
                }

                if (InDefaultPath.IsEmpty() != true)
                {
                    FString DefaultPathString = InDefaultPath;

                    FPaths::MakePlatformFilename(DefaultPathString);

                    IShellItem* DefaultFolder = NULL;
                    HRESULT DefaultPathResult = SHCreateItemFromParsingName(*DefaultPathString, nullptr, IID_PPV_ARGS(&DefaultFolder));

                    if (SUCCEEDED(DefaultPathResult))
                    {
                        SaveFileDialog->SetFolder(DefaultFolder);
                        DefaultFolder->Release();
                    }
                }

                HWND WindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());
                SaveDialogInstance = SaveFileDialog->Show(WindowHandle);

                // Dialog didn't show up.
                if (SUCCEEDED(SaveDialogInstance))
                {
                    SaveFileDialog->GetResult(&ShellItem);
                    
                    UINT FileTypeIndex = 0;
                    SaveFileDialog->GetFileTypeIndex(&FileTypeIndex);
                    FString ExtensionString = ExtensionValues[FileTypeIndex - 1];
                    
                    TArray<FString> ExtensionParts;
                    ExtensionString.ParseIntoArray(ExtensionParts, TEXT("."), true);

                    PWSTR pFileName;
                    ShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pFileName);

                    FString FilePath = FString(pFileName) + TEXT(".") + ExtensionParts[1];

                    if (bIsNormalizeOutputs == true)
                    {
                        FPaths::NormalizeFilename(FilePath);
                    }

                    ShellItem->Release();
                    SaveFileDialog->Release();
                    CoUninitialize();

                    AsyncTask(ENamedThreads::GameThread, [DelegateSaveFile, FilePath]()
                        {
                            DelegateSaveFile.ExecuteIfBound(true, FilePath);
                        }
                    );
                }

                else
                {
                    AsyncTask(ENamedThreads::GameThread, [DelegateSaveFile]()
                        {
                            DelegateSaveFile.ExecuteIfBound(false, TEXT(""));
                        }
                    );
                }
            }

            // Function couldn't create dialog.
            else
            {
                AsyncTask(ENamedThreads::GameThread, [DelegateSaveFile]()
                    {
                        DelegateSaveFile.ExecuteIfBound(false, TEXT(""));
                    }
                );
            }
        }
    );
}