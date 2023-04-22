// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PreOpenCVHeaders.h"
#include "OpenCVHelper.h"

#include <ThirdParty/OpenCV/include/opencv2/imgproc.hpp>
#include <ThirdParty/OpenCV/include/opencv2/highgui/highgui.hpp>

#include <ThirdParty/OpenCV/include/opencv2/core.hpp>
#include "PostOpenCVHeaders.h"
#include "NeuralNetwork.h"

#include "NNI_CNN.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNNI_CNN : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNNI_CNN();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	

public:	
	UPROPERTY(Transient)
		UNeuralNetwork* Network = nullptr;
	float RunModel(cv::Mat image);
	cv::Mat PreProcessImage(cv::Mat image);
		
};
