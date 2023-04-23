// Fill out your copyright notice in the Description page of Project Settings.

#include "Brain.h"
#include "NNI_CNN.h"
#include "VehiclePawn.h"
#include "TrainingDataCapturer.h"

UTrainingDataCapturer::UTrainingDataCapturer()
{
    ImageFilePath = FPaths::ProjectSavedDir() / TEXT("ScreenShots/CameraView");
    extension = TEXT("jpeg");

	NNIInterface = CreateDefaultSubobject<UNNI_CNN>("NNIInterface");
}

float UTrainingDataCapturer::RunPrediction()
{
	FString photoPath = FString::Printf(TEXT("%s_%d%d.%s"), *ImageFilePath, PersonalId, ImageId++, *extension);
	CaptureScene();
	TArray<FColor> colorDataArray;
	//read the pixels
	FRenderTarget* renderTargetResource = TextureTarget->GameThread_GetRenderTargetResource();
	renderTargetResource->ReadPixels(colorDataArray); //BGRA 8
	//create a mat with the data from pixels 
	cv::Mat colorData = cv::Mat(cv::Size(renderTargetResource->GetSizeXY().X, renderTargetResource->GetSizeXY().Y), CV_8UC4, colorDataArray.GetData());
	//TArray<float> in = NNIInterface->PreProcessImage(colorData);
	float result = NNIInterface->RunModel(colorData);
	UE_LOG(LogTemp, Warning, TEXT("Predicted steering: %f"), result);

	return 0.0f;
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

    RenderTarget = NewObject<UTextureRenderTarget2D>();
    RenderTarget->InitCustomFormat(VideoWidth, VideoHeight, PF_B8G8R8A8, false); //BGRA
    TextureTarget = RenderTarget;

	RunPrediction();
}
void UTrainingDataCapturer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    SendTrainingData();
}

void UTrainingDataCapturer::SendTrainingData()
{
	//save image to disk 	
	FString photoPath = FString::Printf(TEXT("%s_%d%d.%s"), *ImageFilePath, PersonalId, ImageId++, *extension);

	// Read the captured data and create an image
	CaptureScene();
	TArray<FColor> bitmap;

	//read pixels
	FRenderTarget* renderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!renderTargetResource->ReadPixels(bitmap))
	{
		return;
	}

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


