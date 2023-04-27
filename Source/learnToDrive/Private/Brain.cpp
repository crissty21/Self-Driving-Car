// Fill out your copyright notice in the Description page of Project Settings.


#include "Brain.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "LandscapeSplinesComponent.h"
#include "Landscape.h"
#include "IImageWrapperModule.h"
#include "Road.h"


ABrain::ABrain()
{
	// Enable tick and set the tick interval
	PrimaryActorTick.bCanEverTick = true;
	ImageFormat = EImageFormat::JPEG;
	CsvFilePath = FPaths::ProjectSavedDir() / TEXT("Data.csv");
}
void ABrain::BeginPlay()
{
	Super::BeginPlay();

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = imageWrapperModule.CreateImageWrapper(ImageFormat);


	USplineComponent* spline = GetSplineFromRoad();
	CreateSplineFromLandscape(spline);
}

USplineComponent* ABrain::GetSplineFromRoad()
{
	ARoad* road = (ARoad*)UGameplayStatics::GetActorOfClass(GetWorld(), ARoad::StaticClass());
	if (road == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find a road in world! (Brain.cpp)"));
		return nullptr;
	}
	return road->SplineComp;
}

void ABrain::CreateSplineFromLandscape(USplineComponent* spline)
{
	//////
	// Data gather 
	//////

	//get the road from the landscape
	ALandscape* landscape = (ALandscape*)UGameplayStatics::GetActorOfClass(GetWorld(), ALandscape::StaticClass());
	ULandscapeSplinesComponent* landscapeSplineComponent = (ULandscapeSplinesComponent*)landscape->GetComponentByClass(ULandscapeSplinesComponent::StaticClass());
	if (landscapeSplineComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No Road On this landscape"));
		return;
	}

	//get the spline secments from the newly found road 
	TArray<USceneComponent*> sceneComponents;
	landscapeSplineComponent->GetChildrenComponents(false, sceneComponents);

	struct FSplinePointInfo
	{
		FVector WorldLocation;
		FVector Tangent;
		FVector Scale;
		FRotator Rotation;
	};
	TArray<FSplinePointInfo> SplinePointsInfo;
	SplinePointsInfo.Reserve(sceneComponents.Num());
	int32 index = 0;
	for (USceneComponent* it : sceneComponents)
	{
		//drop hald of the points 
		if (index++ % 2 == 0)
		{
			continue;
		}
		USplineMeshComponent* splineCom = (USplineMeshComponent*)it;
		if (splineCom != nullptr)
		{
			FSplinePointInfo PointInfo;
			PointInfo.WorldLocation = splineCom->GetComponentLocation();
			PointInfo.Tangent = splineCom->GetStartTangent();
			PointInfo.Rotation = splineCom->GetComponentRotation();
			PointInfo.Scale = splineCom->GetComponentScale();
			SplinePointsInfo.Add(PointInfo);
		}
	}


	//////
	// Data sorter
	//////

	// Find the new order of spline points based on the closest distance (sort the list)
	TArray<FSplinePointInfo> NewSplinePointsOrder;
	NewSplinePointsOrder.Reserve(sceneComponents.Num());
	NewSplinePointsOrder.Add(SplinePointsInfo[0]);
	SplinePointsInfo.RemoveAt(0);

	while (SplinePointsInfo.Num() > 0)
	{
		FSplinePointInfo& LastPoint = NewSplinePointsOrder.Last();
		float MinDistanceSquared = FLT_MAX;
		int32 ClosestPointIndex = INDEX_NONE;

		for (int32 i = 0; i < SplinePointsInfo.Num(); ++i)
		{
			float DistanceSquared = FVector::DistSquared(LastPoint.WorldLocation, SplinePointsInfo[i].WorldLocation);
			if (DistanceSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistanceSquared;
				ClosestPointIndex = i;
			}
		}

		if (ClosestPointIndex != INDEX_NONE)
		{
			NewSplinePointsOrder.Add(SplinePointsInfo[ClosestPointIndex]);
			SplinePointsInfo.RemoveAt(ClosestPointIndex);
		}
	}


	//////
	// new spline creation 
	//////

	spline->SetWorldLocation(landscapeSplineComponent->GetComponentLocation());
	spline->ClearSplinePoints();

	for (int32 i = 0; i < NewSplinePointsOrder.Num(); ++i)
	{
		spline->AddSplineWorldPoint(NewSplinePointsOrder[i].WorldLocation);
		spline->SetTangentAtSplinePoint(i, NewSplinePointsOrder[i].Tangent, ESplineCoordinateSpace::World, false);
		spline->SetRotationAtSplinePoint(i, NewSplinePointsOrder[i].Rotation, ESplineCoordinateSpace::World, false);
		spline->SetScaleAtSplinePoint(i, NewSplinePointsOrder[i].Scale, false);
	}

	spline->UpdateSpline();
}

void ABrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bSaveTrainingData)
	{
		SaveTrainingData();
	}
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