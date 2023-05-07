#include "VehiclePawn.h"
#include "DrawDebugHelpers.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "SpeedChange.h"

#include "Brain.h"

AVehiclePawn::AVehiclePawn()
{
	// setup components
	PrimaryActorTick.bCanEverTick = true;
	SetUpComponents();
	PrevSpeedError = 30.f;
}


void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	SetUpTrainingDataCapturer();
	SetUpChaosWheeledVehicleComponent();
	SetUpRoad();
	BreakLights(false);
	SetUpInput();
}

void AVehiclePawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor && OtherActor != this)
	{
		ASpeedChange* TriggerVolume = Cast<ASpeedChange>(OtherActor);
		if (TriggerVolume)
		{
			DesiredSpeed = TriggerVolume->NewSpeed;
			ChangeMaxSpeedDisplay(DesiredSpeed);
		}
	}
}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (DrivingStile == Driving::FollowSpline)
	{
		if (FollowedSpline)
		{
			KeepRoad();
		}
	}
	else if (DrivingStile == Driving::RunNetwork)
	{
		Steer(steerFromNN);
	}
	CruiseControll(DeltaTime);

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

void AVehiclePawn::SetUpComponents()
{
	FrontPoint = CreateDefaultSubobject<USceneComponent>("FrontPoint");
	BackPoint = CreateDefaultSubobject<USceneComponent>("BackPoint");
	AdvancePoint = CreateDefaultSubobject<USceneComponent>("AdvancePoint");

	FrontPoint->SetupAttachment(RootComponent);
	BackPoint->SetupAttachment(RootComponent);
	AdvancePoint->SetupAttachment(RootComponent);

	FrontPoint->SetRelativeLocation(FVector(130, 0, 0));
	BackPoint->SetRelativeLocation(FVector(-125, 0, 0));
	AdvancePoint->SetRelativeLocation(FVector(400, 0, 0));
}

void AVehiclePawn::SetUpRoad()
{
	ARoad* Road = GetClosestRoad();
	if (Road)
	{
		FollowedSpline = Road->SplineComp;
	}
}

void AVehiclePawn::SetUpChaosWheeledVehicleComponent()
{
	ChaosWheeledVehicleComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
	if (ChaosWheeledVehicleComponent == nullptr)
	{
		UE_LOG(LogTemp, Fatal, TEXT("Invalid cast to UChaosWheeledVehicleMovementComponent in VehiclePawn.cpp"));
	}
}

void AVehiclePawn::SetUpInput()
{
	// Ensure the actor has an input component
	if (!InputComponent)
	{
		InputComponent = NewObject<UInputComponent>(this);
		InputComponent->RegisterComponent();
	}

	// Bind input events
	InputComponent->BindAction("Predict", IE_Pressed, this, &AVehiclePawn::Predict);
	if (DrivingStile == Driving::UserControll)
	{
		InputComponent->BindAxis("MoveForward", this, &AVehiclePawn::HandleForwardInput);
		InputComponent->BindAxis("MoveRight", this, &AVehiclePawn::Steer);	
		InputComponent->BindAction("HandBreak", IE_Pressed, this, &AVehiclePawn::HandBreak);
		InputComponent->BindAction("HandBreak", IE_Released, this, &AVehiclePawn::HandBreakReleased);

		DesiredSpeed = 0;
	}
}

void AVehiclePawn::SetUpTrainingDataCapturer()
{
	TrainingDataCapturer = NewObject<UTrainingDataCapturer>(this, UTrainingDataCapturer::StaticClass(), TEXT("ImageProcesor"));
	TrainingDataCapturer->RegisterComponent();
	TrainingDataCapturer->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	TrainingDataCapturer->Parent = this;
	TrainingDataCapturer->PersonalId = PersonalID;
	if (DrivingStile == Driving::RunNetwork)
	{
		TrainingDataCapturer->bRunModel = true;
	}
	else
	{
		TrainingDataCapturer->bRunModel = false;
	}
	TrainingDataCapturer->bCaptureData = bCaptureData;
	TrainingDataCapturer->PrimaryComponentTick.TickInterval = 1.0f / TickingFreq;

	TrainingDataCapturer->Init();
}

void AVehiclePawn::Predict()
{
	if (DrivingStile == Driving::RunNetwork)
	{
		float out = TrainingDataCapturer->GetModelOutput();
		UE_LOG(LogTemp, Warning, TEXT("Out: %f"), out);
		UE_LOG(LogTemp, Warning, TEXT("Real: %f"), GetSteering());
	}
}

void AVehiclePawn::HandleForwardInput(float value)
{
	if (DesiredSpeed + value * Acceleration > MaxReverse &&
		DesiredSpeed + value * Acceleration < MaxSpeed)
	{
		DesiredSpeed += value * Acceleration;
		
	}
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
	//UE_LOG(LogTemp, Warning, TEXT("value: %f , real: % f"), value, ChaosWheeledVehicleComponent->GetSteeringInput())
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
