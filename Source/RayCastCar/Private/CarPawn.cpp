// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "CarWheelSceneComponent.h"

// Sets default values
ACarPawn::ACarPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BodyMesh"));
	RootComponent = RootMesh;
	RootMesh->SetSimulatePhysics(true);

	CarWheels.SetNum(4);
	CarWheels[0] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentFL"));
	
	CarWheels[0]->SetupAttachment(RootMesh);
	TSet<TObjectPtr<UActorComponent>> NewGrandchildComponents = CarWheels[0]->CreateChildComponents(ObjectInitializer, this);
	GrandchildComponents.Append(NewGrandchildComponents);
	CarWheels[0]->AttachChildComponents();

	//WheelMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("WheelMesh"));
	//WheelMesh->SetupAttachment(CarWheels[0]);

}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

