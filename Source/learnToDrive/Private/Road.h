// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Road.generated.h"

UCLASS()
class ARoad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USplineComponent* SplineComp = nullptr;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
