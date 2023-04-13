// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Saver.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FThresholdSave
{
	GENERATED_BODY()
	UPROPERTY()
		FVector2D Threshold = FVector2D(0);
	UPROPERTY()
		bool UseThreshold = false;
};
UCLASS()
class USaver : public USaveGame
{
	GENERATED_BODY()
	
public:
	USaver();

	UPROPERTY()
		TArray<FThresholdSave> ThresholdsToBeSaved;

	void addToSaveData(const FVector2D Threshold,const bool useThreshold);


};
