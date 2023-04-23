// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "TrainingDataCapturer.generated.h"


UCLASS()
class UTrainingDataCapturer : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UTrainingDataCapturer();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;

	int8 PersonalId = 0;

	class AVehiclePawn* Parent = nullptr;

	int8 TickingFreq = 1;

	UFUNCTION()
		float RunPrediction();

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly)
		int32 VideoWidth;
	UPROPERTY(EditDefaultsOnly)
		int32 VideoHeight;

private:
	UPROPERTY()
		UTextureRenderTarget2D* RenderTarget = nullptr;
	UPROPERTY()
		FString ImageFilePath;
	UPROPERTY()
		FString extension;
	UPROPERTY()
		class UNNI_CNN* NNIInterface = nullptr;

	int32 ImageId = 0;
	float DT;
	class ABrain* gameMode = nullptr;

	void SendTrainingData();
};
