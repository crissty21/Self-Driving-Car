#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Road.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "TrainingDataCapturer.h"

#include "VehiclePawn.generated.h"

UCLASS()
class AVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	AVehiclePawn();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
		void BreakLights(bool state);

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Movement")
		void MoveForward(float value);
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void Steer(float value);
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void HandBreak();
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void HandBreakReleased();
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void ResetCar();
	UFUNCTION()
		float GetSteering();
	UFUNCTION()
		float GetThrottle();
	UFUNCTION()
		float GetBreak();
	UFUNCTION()
		float GetSpeed();

	UPROPERTY(EditAnywhere)
		int8 PersonalID = 0;

protected:
	UPROPERTY(EditAnywhere)
		USceneComponent* FrontPoint = nullptr;
	UPROPERTY(EditAnywhere)
		USceneComponent* BackPoint = nullptr;
	UPROPERTY(EditAnywhere)
		USceneComponent* AdvancePoint = nullptr;

	UPROPERTY(EditAnywhere)
		UTrainingDataCapturer* TrainingDataCapturer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DesiredSpeed = 80.f;
	UPROPERTY(EditDefaultsOnly)
		int8 TickingFreq = 1;
	UPROPERTY(EditDefaultsOnly)
		float CriticalAngle = 0.5;
	UPROPERTY(EditDefaultsOnly)
		float MaxSpeed = 100;
	UPROPERTY(EditDefaultsOnly)
		float BreakTolerance = 0.5f;
	UPROPERTY(EditDefaultsOnly)
		bool DrawLine = false;


private:

	UPROPERTY()
		class USplineComponent* FollowedSpline = nullptr;
	UPROPERTY()
		class UChaosWheeledVehicleMovementComponent* ChaosWheeledVehicleComponent = nullptr;
	UFUNCTION()
		ARoad* GetClosestRoad();
	bool CustomBreakSis = false;
	//PID constants 
	float Kp = 1.0f;
	float Ki = 0.0f;
	float Kd = 0.0f;
	
	float PrevSpeedError = 30.f;

	float IntegralError = 0.0f;

	bool BreakLightsState = false;
	
	void CruiseControll(float DeltaTime);
	
	void KeepRoad();
};
