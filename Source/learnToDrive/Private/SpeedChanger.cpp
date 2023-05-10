#include "VehiclePawn.h"
#include "SpeedChanger.h"

// Sets default values
ASpeedChanger::ASpeedChanger()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TriggerBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox1"));
	TriggerBox1->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	TriggerBox1->SetBoxExtent(FVector(50.f, 600.f, 100.f));
	TriggerBox1->SetRelativeLocation(FVector(50.f, 0.f, 0.f));

	TriggerBox2 = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox2"));
	TriggerBox2->SetupAttachment(Root);
	TriggerBox2->SetBoxExtent(FVector(50.f, 600.f, 100.f)); 
	TriggerBox2->SetRelativeLocation(FVector(-50.f, 0.f, 0.f));


	// Create and attach ArrowComponent to root component
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(Root);

	TriggerBox1->ShapeColor = FColor::Red;
	TriggerBox2->ShapeColor = FColor::Blue;

}

float ASpeedChanger::GetNewSpeed()
{
	TArray<AActor*> overlappingActors;
	TriggerBox1->GetOverlappingActors(overlappingActors, AVehiclePawn::StaticClass());
	if (overlappingActors.Num() > 0)
	{
		return FrontSpeed;
	}
	else
	{
		return BackSpeed;
	}
}

void ASpeedChanger::BeginPlay()
{
	Super::BeginPlay();

}


