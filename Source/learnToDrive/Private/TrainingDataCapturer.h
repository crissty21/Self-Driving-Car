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

	bool bCaptureData = false;
	bool bRunModel = false;

	float GetModelOutput();
	void Init();

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly)
		int32 VideoWidth;
	UPROPERTY(EditDefaultsOnly)
		int32 VideoHeight;

private:
	UPROPERTY()
		FString ImageFilePath;
	UPROPERTY()
		FString extension;
	UFUNCTION()
		TArray<FColor> ReadCamera();

	int32 ImageId = 0;
	UPROPERTY()
		class ABrain* gameMode = nullptr;
	UPROPERTY()
		class UNNI_CNN* NeuralNetwork = nullptr;
	void SendTrainingData();
	
};
