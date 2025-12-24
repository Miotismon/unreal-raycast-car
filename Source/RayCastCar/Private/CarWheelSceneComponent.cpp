// Fill out your copyright notice in the Description page of Project Settings.


#include "CarWheelSceneComponent.h"

// Sets default values for this component's properties
UCarWheelSceneComponent::UCarWheelSceneComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//MeshHolder = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("MeshHolder"));
	//CaliperMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("CaliperMesh"));
	//WheelMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("WheelMesh"));
	
}

TSet<TObjectPtr<UActorComponent>> UCarWheelSceneComponent::CreateChildComponents(const FObjectInitializer& ObjectInitializer, UObject* Parent)
{
	TSet<TObjectPtr<UActorComponent>> NewChildSet;
	
	WheelMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(Parent, TEXT("WheelMesh"));
	
	NewChildSet.Add(WheelMesh);
	return NewChildSet;
}

void UCarWheelSceneComponent::AttachChildComponents()
{
	//WheelMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(Parent, TEXT("WheelMesh"));
	WheelMesh->SetupAttachment(this);
}


// Called when the game starts
void UCarWheelSceneComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCarWheelSceneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Called when Component gets registered
void UCarWheelSceneComponent::OnRegister()
{
	Super::OnRegister();

	// this is here bcus attaching subcomponents in the constructor (before this component is attached to it's parent) is bad
	//MeshHolder->SetupAttachment(this);
	//CaliperMesh->SetupAttachment(MeshHolder);
	//WheelMesh->SetupAttachment(MeshHolder);
	//WheelMesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

	//UE_LOG(LogTemp, Warning, TEXT("I'M ABOUT TO BLOW UP"));
}
