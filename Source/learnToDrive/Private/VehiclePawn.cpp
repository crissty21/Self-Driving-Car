#include "VehiclePawn.h"
#include "DrawDebugHelpers.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"

#include "Brain.h"

AVehiclePawn::AVehiclePawn()
{
	// setup components
	PrimaryActorTick.bCanEverTick = true;
	FrontPoint = CreateDefaultSubobject<USceneComponent>("FrontPoint");
	BackPoint = CreateDefaultSubobject<USceneComponent>("BackPoint");
	AdvancePoint = CreateDefaultSubobject<USceneComponent>("AdvancePoint");

	FrontPoint->SetupAttachment(RootComponent);
	BackPoint->SetupAttachment(RootComponent);
	AdvancePoint->SetupAttachment(RootComponent);

	FrontPoint->SetRelativeLocation(FVector(130, 0, 0));
	BackPoint->SetRelativeLocation(FVector(-125, 0, 0));
	AdvancePoint->SetRelativeLocation(FVector(400, 0, 0));


	TrainingDataCapturer = CreateDefaultSubobject<UTrainingDataCapturer>("ImageProcesor");
	TrainingDataCapturer->SetupAttachment(RootComponent);

	TrainingDataCapturer->SetRelativeLocation(FVector(142, 0, 150));
	TrainingDataCapturer->SetRelativeRotation(FRotator(-10, 0, 0));
	TrainingDataCapturer->FOVAngle = 120;
	TrainingDataCapturer->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	TrainingDataCapturer->bCaptureEveryFrame = false;
	TrainingDataCapturer->Parent = this;
	TrainingDataCapturer->PrimaryComponentTick.TickInterval = 1.0f / TickingFreq;
	TrainingDataCapturer->PersonalId = PersonalID;
}

void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	ChaosWheeledVehicleComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
	if (ChaosWheeledVehicleComponent == nullptr)
	{
		UE_LOG(LogTemp, Fatal, TEXT("Invalid cast to UChaosWheeledVehicleMovementComponent in VehiclePawn.cpp"));
	}

	ARoad* Road = GetClosestRoad();
	if (Road)
	{
		FollowedSpline = Road->SplineComp;
	}

	PrevSpeedError = 30.f;
	BreakLights(false);
}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (FollowedSpline)
	{
		KeepRoad();
		CruiseControll(DeltaTime);
	}
	else
	{
		if (bRunModel)
		{
			CruiseControll(DeltaTime);

		}
	}
}

void AVehiclePawn::CruiseControll(float DeltaTime)
{
	//current speed
	float KPH = GetVehicleMovement()->GetForwardSpeed() * 0.036f;
	//error
	float errorKPH = DesiredSpeed - KPH;


	// Proportional term
	float p = Kp * errorKPH;

	// Integral term
	IntegralError += errorKPH * DeltaTime;
	float i = Ki * IntegralError;

	// Derivative term
	float derivativeError = (errorKPH - PrevSpeedError) / DeltaTime;
	float d = Kd * derivativeError;
	//derivate

	PrevSpeedError = errorKPH;

	float value = FMath::Clamp(p + i + d, -1.0f, 1.0f);

	MoveForward(value);
}

void AVehiclePawn::KeepRoad()
{

	FVector advancePointCoordinates = AdvancePoint->GetComponentLocation();
	FVector frontPointCoordinates = FrontPoint->GetComponentLocation();
	FVector backPointCoordinates = BackPoint->GetComponentLocation();

	//get closest point on spline
	FVector coordOnSpline = FollowedSpline->FindLocationClosestToWorldLocation(advancePointCoordinates, ESplineCoordinateSpace::World);
	coordOnSpline.Z = advancePointCoordinates.Z;

	if (DrawLine)
	{
		DrawDebugLine(GetWorld(), frontPointCoordinates, coordOnSpline, FColor::Red, false, -1, 0, 10);
	}

	FVector L = frontPointCoordinates - backPointCoordinates;
	FVector ld = coordOnSpline - backPointCoordinates;
	float a = L.HeadingAngle() - ld.HeadingAngle();

	float LDist = FVector::Dist(backPointCoordinates, frontPointCoordinates);
	float angle = (atan((sin(a) * LDist * 2) / ld.Size()));

	ChaosWheeledVehicleComponent->SetSteeringInput(-angle * 2);

	//set desired speed in order to be able to take coreners
	float curentSpeed = GetVehicleMovement()->GetForwardSpeed() * 0.036;
	if (FMath::Abs(angle) * 2 >= CriticalAngle)
	{
		float update = MaxSpeed - (MaxSpeed - 10) * FMath::Abs(angle) * 2;
		if (curentSpeed - update >= 10)
			DesiredSpeed = update;
	}
	else
		DesiredSpeed = MaxSpeed;

}

void AVehiclePawn::MoveForward(float value)
{
	if (ChaosWheeledVehicleComponent->GetHandbrakeInput())
		return;

	ChaosWheeledVehicleComponent->SetThrottleInput(value);
	//turn on break lights
	if (BreakLightsState == true)
	{
		BreakLightsState = false;
		BreakLights(false);
	}

}

void AVehiclePawn::Steer(float value)
{
	ChaosWheeledVehicleComponent->SetSteeringInput(value);
}

void AVehiclePawn::HandBreak()
{
	ChaosWheeledVehicleComponent->SetHandbrakeInput(true);
}

void AVehiclePawn::HandBreakReleased()
{
	ChaosWheeledVehicleComponent->SetHandbrakeInput(false);
}

void AVehiclePawn::ResetCar()
{
	FVector desiredLocation = GetActorLocation();
	desiredLocation.Z += 50;
	FRotator desiredRotation = FRotator::ZeroRotator;
	desiredRotation.Yaw = GetActorRotation().Yaw;

	SetActorLocation(desiredLocation, true);
	SetActorRotation(desiredRotation);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

ARoad* AVehiclePawn::GetClosestRoad()
{
	TArray<AActor*> roads;
	UGameplayStatics::GetAllActorsOfClass(this, ARoad::StaticClass(), roads);

	if (roads.IsEmpty())
	{
		return nullptr;
	}

	FVector myLocation = GetActorLocation();
	AActor* closestActor = nullptr;

	float closestDistance = TNumericLimits<float>::Max();

	for (AActor* actor : roads)
	{
		FVector actorLocation = actor->GetActorLocation();
		float distance = FVector::Distance(actorLocation, myLocation);
		if (distance < closestDistance)
		{
			closestActor = actor;
			closestDistance = distance;
		}
	}
	if (closestActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find any actor of class ARoad"));
		return nullptr;
	}
	return (ARoad*)closestActor;
}

float AVehiclePawn::GetSteering()
{
	if (ChaosWheeledVehicleComponent == nullptr)
	{
		return 0;
	}
	return ChaosWheeledVehicleComponent->GetSteeringInput();
}

float AVehiclePawn::GetThrottle()
{
	if (ChaosWheeledVehicleComponent == nullptr)
	{
		return 0;
	}
	return ChaosWheeledVehicleComponent->GetThrottleInput();
}

float AVehiclePawn::GetBreak()
{
	if (ChaosWheeledVehicleComponent == nullptr)
	{
		return 0;
	}
	return ChaosWheeledVehicleComponent->GetBrakeInput();
}

float AVehiclePawn::GetSpeed()
{
	if (ChaosWheeledVehicleComponent == nullptr)
	{
		return 0;
	}
	return ChaosWheeledVehicleComponent->GetForwardSpeedMPH();
}
