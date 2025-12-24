// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CarWheelSceneComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RAYCASTCAR_API UCarWheelSceneComponent : public USceneComponent
{
	GENERATED_BODY()


public:
	UPROPERTY(VisibleAnywhere, Category = "Suspension")
	float RayCastDistance;
	UPROPERTY(VisibleAnywhere, Category = "Suspension")
	float RestDistance;
	UPROPERTY(VisibleAnywhere, Category = "Suspension")
	float SpringStrength;
	UPROPERTY(VisibleAnywhere, Category = "Suspension")
	float SpringDamping;

	UPROPERTY(VisibleAnywhere, Category = "Steering")
	bool IsSteering;
	UPROPERTY(VisibleAnywhere, Category = "Steering")
	float TireGripFactor;
	UPROPERTY(VisibleAnywhere, Category = "Steering")
	float RollingFrictionFactor;
	UPROPERTY(VisibleAnywhere, Category = "Steering")
	float TireMass;

	UPROPERTY(VisibleAnywhere, Category = "Drive")
	bool IsDrive;
	UPROPERTY(VisibleAnywhere, Category = "Brake")
	bool IsBrake;

	/*UPROPERTY(VisibleAnywhere, Category = "Meshes")
	TObjectPtr<USceneComponent> MeshHolder;

	UPROPERTY(VisibleAnywhere, Category = "Meshes")
	TObjectPtr<UStaticMeshComponent> CaliperMesh;*/

	UPROPERTY(VisibleAnywhere, Category = "Meshes")
	TObjectPtr<UStaticMeshComponent> WheelMesh;
	UPROPERTY(VisibleAnywhere, Category = "Meshes")
	float WheelMeshDiameter;

	



public:	
	// Sets default values for this component's properties
	UCarWheelSceneComponent(const FObjectInitializer& ObjectInitializer);

	TSet<TObjectPtr<UActorComponent>> CreateChildComponents(const FObjectInitializer& ObjectInitializer, UObject* Parent);
	void AttachChildComponents();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called when Component gets registered
	virtual void OnRegister() override;
};
