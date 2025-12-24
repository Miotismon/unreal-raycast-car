// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CarPawn.generated.h"



UCLASS()
class RAYCASTCAR_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RootMesh;


protected:

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<class UCarWheelSceneComponent>> CarWheels;

	/*UPROPERTY(VisibleAnywhere)
	TObjectPtr<UActorComponent> WheelMesh;*/

	UPROPERTY(VisibleAnywhere)
	TSet<TObjectPtr<UActorComponent>> GrandchildComponents;


public:
	// Sets default values for this pawn's properties
	ACarPawn(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
