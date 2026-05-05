// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "CarPawn.generated.h"


class UCarWheelSceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UCarAnimInstance;

UCLASS()
class RAYCASTCAR_API ACarPawn : public APawn
{
    GENERATED_BODY()

public:
    // Apparently, if you add a component from C++ with any of the visible specifiers, like VisibleDefaultsOnly, it will show all of that component’s properties within that actor’s details panel. (https://forums.unrealengine.com/t/blueprint-details-panel-is-showing-all-child-components/780426/7) 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MainBody")
    TObjectPtr<USkeletalMeshComponent> RootMesh;

    UPROPERTY(VisibleAnywhere, Category = "Wheels")
    TArray<TObjectPtr<UCarWheelSceneComponent>> CarWheels;

    UPROPERTY(EditAnywhere, Category = "Camera")
    TObjectPtr<USpringArmComponent> SpringArm;
    
    UPROPERTY(EditAnywhere, Category = "Camera")
    TObjectPtr<UCameraComponent> Camera;


    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SuspensionTopOffset;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SuspensionMaxDistance;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float RestDistance;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringStrength;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringDamping;


    UPROPERTY(EditAnywhere, Category = "Steering")
    float MaxSteeringAngle;
    UPROPERTY(EditAnywhere, Category = "Steering")
    float TireGripFactor;
    UPROPERTY(EditAnywhere, Category = "Steering")
    float RollingFrictionFactor;
    UPROPERTY(EditAnywhere, Category = "Steering")
    float TireMass;
    UPROPERTY(EditAnywhere, Category = "Steering")
    TObjectPtr<UCurveFloat> GripCurve;


    UPROPERTY(EditAnywhere, Category = "Drive")
    float TopSpeed;
    UPROPERTY(EditAnywhere, Category = "Drive")
    float MaxTorque;
    UPROPERTY(EditAnywhere, Category = "Drive")
    TObjectPtr<UCurveFloat> PowerCurve;
    
    UPROPERTY(EditAnywhere, Category = "Brake")
    float BrakingStrength;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    FRotator CameraTargetRotation;
    UPROPERTY(EditAnywhere, Category = "Camera")
    FRotator CameraBaseRotation;
    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraSmoothingRate = 0.01f;

    UPROPERTY(VisibleAnywhere, Category = "Input")
    float InputThrottle;
    UPROPERTY(VisibleAnywhere, Category = "Input")
    float InputBrake;
    UPROPERTY(VisibleAnywhere, Category = "Input")
    float InputHandbrake;
    UPROPERTY(VisibleAnywhere, Category = "Input")
    float InputSteering;
    UPROPERTY(VisibleAnywhere, Category = "Input")
    float InterpolatedSteering;
    UPROPERTY(VisibleAnywhere, Category = "Input")
    FVector2D InputCamera;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> CarMappingContext;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Throttle;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Brake;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Handbrake;
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Steer;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Camera;

private:
    UPROPERTY()
    TObjectPtr<UCarAnimInstance> CarAnimInstance;

public:
    // Sets default values for this pawn's properties
    ACarPawn(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    void OnThrottleInput(float Value);

    UFUNCTION(BlueprintCallable)
    void OnBrakeInput(float Value);

    UFUNCTION(BlueprintCallable)
    void OnHandbrakeInput(bool bPressed);

    UFUNCTION(BlueprintCallable)
    void OnSteerInput(float Value);

    UFUNCTION(BlueprintCallable)
    void OnCameraInput(FVector2D Value);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;


public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    //virtual void PossessedBy(AController* NewController) override;

    // Called to bind functionality to input
    //virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    void UpdateAnimWheelData(float DeltaTime);

};
