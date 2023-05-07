// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "SpeedChange.generated.h"

/**
 * 
 */
UCLASS()
class LEARNTODRIVE_API ASpeedChange : public ATriggerBox
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		float NewSpeed = 50.f;
};
