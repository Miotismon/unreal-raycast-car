// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CarWheelSceneComponent.generated.h"

class ACarPawn;


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

	UPROPERTY(EditAnywhere, Category = "Steering")
	bool IsSteering;
	UPROPERTY(VisibleAnywhere, Category = "Steering")
	float TireGripFactor;
	UPROPERTY(VisibleAnywhere, Category = "Steering")
	float RollingFrictionFactor;
	UPROPERTY(VisibleAnywhere, Category = "Steering")
	float TireMass;

	UPROPERTY(EditAnywhere, Category = "Drive")
	bool IsDrive;
	UPROPERTY(EditAnywhere, Category = "Brake")
	bool IsBrake;
	UPROPERTY(EditAnywhere, Category = "Brake")
	bool IsHandbrake;

	UPROPERTY()
	TObjectPtr<USceneComponent> MeshHolder;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CaliperMesh;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WheelMesh;
	UPROPERTY(EditAnywhere, Category = "WheelMesh")
	float WheelMeshRadius = 30.0f;

	// ptr back to the body for physics calculations
	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> BodyMesh;

public:	
	// Sets default values for this component's properties
	UCarWheelSceneComponent(const FObjectInitializer& ObjectInitializer);

	// this is here bcus creating and attaching subcomponents in the constructor of an actors component (which happens before this component is attached to it's parent) is bad
	TSet<TObjectPtr<USceneComponent>> CreateChildComponents(const FObjectInitializer& ObjectInitializer, UObject* Parent);
	void AttachChildComponents();

	void CalculateAndApplyForces(float DeltaTime);
	
private:
	bool LineTraceToGround(FHitResult& OutHit);
	FVector CalculateSuspensionForce(float LineTraceDistance);
	FVector CalculateGripForce(float DeltaTime);
	FVector CalculateAccelerationForce(FVector ImpactNormal);
	FVector CalculateBrakingForce(float DeltaTime);
	void MoveAndRotateWheel(float DeltaTime, float SuspensionLength);
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
