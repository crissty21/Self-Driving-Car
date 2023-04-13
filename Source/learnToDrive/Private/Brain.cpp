// Fill out your copyright notice in the Description page of Project Settings.


#include "Brain.h"

ABrain::ABrain()
{

	// Enable tick and set the tick interval
	PrimaryActorTick.bCanEverTick = true;
	ImageFormat = EImageFormat::JPEG;
	CsvFilePath = FPaths::ProjectSavedDir() / TEXT("Data.csv");
}

void ABrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bSaveTrainingData)
	{
		SaveTrainingData();
	}
}

void ABrain::BeginPlay()
{
	Super::BeginPlay();

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = imageWrapperModule.CreateImageWrapper(ImageFormat);
}

bool ABrain::WriteRowToCSV(const FString& filePath, const TArray<FString>& row)
{
    FString rowLine;
    for (const FString& Cell : row)
    {
        rowLine.Append(Cell + ",");
    }
    // Remove the last comma and add a newline character
    FString dumy = rowLine.LeftChop(1);
    rowLine.Append("\n");

    // Write the row to the file
    return FFileHelper::SaveStringToFile(rowLine, *filePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

bool ABrain::SaveCameraViewToDisk(const FString& filePath, TArray<FColor> bitmap)
{
	// Compress the image

	TArray<uint8> compressedData;
	if (ImageWrapper->SetRaw(bitmap.GetData(), bitmap.GetAllocatedSize(), VideoWidth, VideoHeight, ERGBFormat::BGRA, 8))
	{
		compressedData = ImageWrapper->GetCompressed();
	}
	else
	{
		return false;
	}

	// Save the image to disk
	return FFileHelper::SaveArrayToFile(compressedData, *filePath);
}

void ABrain::SaveTrainingData()
{
	if (photos.IsEmpty() == false)
	{
		TPair<FString, TArray<FColor>>* currentPhoto = photos.Peek();
		bool bSaved = SaveCameraViewToDisk(currentPhoto->Key, currentPhoto->Value);
		if (!bSaved)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save camera view to %s"), *currentPhoto->Key);
		}
		photos.Pop();
	}
	 
	//save data to csv 
	// Generate data and write it to the file
	if (CSVdata.IsEmpty() == false)
	{
		if (!WriteRowToCSV(CsvFilePath, *CSVdata.Peek()))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to write data row to file %s"), *CsvFilePath);
		}
		CSVdata.Pop();
	}
}

void ABrain::AddImageToSave(FString path, TArray<FColor> data)
{
	photos.Enqueue(TPair<FString, TArray<FColor>>(path, data));
}

void ABrain::AddDataToSave(TArray<FString> row)
{
	CSVdata.Enqueue(row);
}