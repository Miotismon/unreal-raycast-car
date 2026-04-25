// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CarAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FWheelAnimData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float WheelZOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float RotationDeg = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float SteeringDeg = 0.0f;
};

UCLASS()
class RAYCASTCAR_API UCarAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
    TArray<FWheelAnimData> WheelData;


    virtual void NativeInitializeAnimation() override;
};
