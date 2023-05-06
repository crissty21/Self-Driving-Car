// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"


#include "NNI_CNN.generated.h"

namespace cv{
	class Mat;
}
UCLASS()
class UNNI_CNN : public UObject
{
	GENERATED_BODY()
public:
	UNNI_CNN();
	UPROPERTY(Transient)
		class UNeuralNetwork* Network = nullptr;
	UFUNCTION()
		float RunModel(TArray<FColor> image, int16 width, int16 height);
	
	TArray<float> PreProcessImage(cv::Mat image);

private:
	UPROPERTY(Transient)
		TArray<float> InArray;
};
