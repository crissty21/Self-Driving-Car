// Fill out your copyright notice in the Description page of Project Settings.

#include "Brain.h"
#include "VehiclePawn.h"
#include "NNI_CNN.h"
#include "TrainingDataCapturer.h"

UTrainingDataCapturer::UTrainingDataCapturer()
{ 
    PrimaryComponentTick.bCanEverTick = true;

    ImageFilePath = FPaths::ProjectSavedDir() / TEXT("ScreenShots/CameraView");
    extension = TEXT("jpeg");
}

void UTrainingDataCapturer::BeginPlay()
{
    Super::BeginPlay();
	

	gameMode = (ABrain*)GetWorld()->GetAuthGameMode();
	if (gameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast gamemode to ABrain"));
		VideoHeight = 1;
		VideoWidth = 1;
	}
	else
	{
		VideoHeight = gameMode->VideoHeight;
		VideoWidth = gameMode->VideoWidth;
	}

	TextureTarget = NewObject<UTextureRenderTarget2D>();
	TextureTarget->InitCustomFormat(VideoWidth, VideoHeight, PF_B8G8R8A8, false);

}

void UTrainingDataCapturer::Init()
{
	SetRelativeLocation(FVector(142, 0, 150));
	SetRelativeRotation(FRotator(-10, 0, 0));
	FOVAngle = 120;
	CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	bCaptureEveryFrame = false;
	if (bRunModel)
	{
		NeuralNetwork = NewObject<UNNI_CNN>();
	}
}

float UTrainingDataCapturer::GetModelOutput()
{
	if (bRunModel == false)return 0;
	if (NeuralNetwork == nullptr)return 0;
	return NeuralNetwork->RunModel(ReadCamera(), VideoWidth, VideoHeight);
	return 0;
}

void UTrainingDataCapturer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bCaptureData)
	{
		SendTrainingData();
	}
	if (bRunModel)
	{

	}
}

TArray<FColor> UTrainingDataCapturer::ReadCamera()
{
	CaptureScene();
	TArray<FColor> bitmap;

	//read pixels
	if (TextureTarget == nullptr)return bitmap;
	FRenderTarget* renderTargetResource = TextureTarget->GameThread_GetRenderTargetResource();
	if (!renderTargetResource->ReadPixels(bitmap))
	{
		return bitmap;
	}
	return bitmap;
}

void UTrainingDataCapturer::SendTrainingData()
{
	//save image to disk 	
	FString photoPath = FString::Printf(TEXT("%s_%d%d.%s"), *ImageFilePath, PersonalId, ImageId++, *extension);

	// Read the captured data and create an image
	
	TArray<FColor> bitmap = ReadCamera();
	if (bitmap.IsEmpty())return;
	gameMode->AddImageToSave(photoPath, bitmap);
	
	//save data to csv 
	// Generate data 
	if (Parent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Parent"));
		return;
	}
	float steering = Parent->GetSteering();
	if (steering != 0)
	{
		steering = FMath::RoundToInt(steering / 0.05f) * 0.05f;
	}
	TArray<FString> dataRow = {
		photoPath,
		FString::SanitizeFloat(steering),
		FString::SanitizeFloat(Parent->GetThrottle()),
		FString::SanitizeFloat(Parent->GetBreak()),
		FString::SanitizeFloat(Parent->GetSpeed())
	};
	gameMode->AddDataToSave(dataRow);
}


