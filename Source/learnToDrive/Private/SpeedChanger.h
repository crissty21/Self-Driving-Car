// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "SpeedChanger.generated.h"

UCLASS()
class ASpeedChanger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpeedChanger();

	float GetNewSpeed();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Components")
		UBoxComponent* TriggerBox1;

	UPROPERTY(EditAnywhere, Category = "Components")
		UBoxComponent* TriggerBox2;

	UPROPERTY(EditAnywhere, Category = "Components")
		UArrowComponent* ArrowComponent;
	UPROPERTY(EditAnywhere, Category = "Components")
		USceneComponent* Root;
	
	UPROPERTY(EditAnywhere)
		float FrontSpeed = 50.f;
	UPROPERTY(EditAnywhere)
		float BackSpeed = 50.f;


};
