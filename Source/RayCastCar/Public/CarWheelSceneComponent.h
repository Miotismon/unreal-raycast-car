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
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SuspensionTopOffset;
    UPROPERTY(VisibleAnywhere, Category = "Suspension")
    float SuspensionMaxDistance;
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

    UPROPERTY(EditAnywhere, Category = "Debug")
    FVector DebugDrawOffset;

    UPROPERTY(EditAnywhere, Category = "WheelMesh")
    FName WheelBoneName = TEXT("");
    UPROPERTY(EditAnywhere, Category = "WheelMesh")
    float WheelMeshRadius = 30.0f;

    // ptr back to the body for physics calculations
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> BodyMesh;

private:
    

    // Reference Skeleton Data
    int32 BoneIndex;
    FTransform RefTransformCS;


public:	
    // Sets default values for this component's properties
    UCarWheelSceneComponent(const FObjectInitializer& ObjectInitializer);

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void CalculateAndApplyForces(float DeltaTime);
    
private:

    // SkeletalMesh setup
    void InitWheelRefData();
    static FTransform GetRefPoseComponentTransform(const FReferenceSkeleton& RefSkel, int32 BoneIndex);

    // Getting world space locations
    FVector GetRefWheelWorldLocation() const;
    FVector GetSuspensionStart() const;

    // Physics Calc
    bool LineTraceToGround(FHitResult& OutHit) const;
    FVector CalculateSuspensionForce(float LineTraceDistance) const;
    FVector CalculateGripForce(float DeltaTime) const;
    FVector CalculateAccelerationForce(FVector ImpactNormal) const;
    FVector CalculateBrakingForce(float DeltaTime) const;
    void MoveAndRotateWheel(float DeltaTime, float SuspensionLength);

public:
    // Animation Data
    float GetDeltaRotationDeg(float DeltaTime) const;
    
    

};
