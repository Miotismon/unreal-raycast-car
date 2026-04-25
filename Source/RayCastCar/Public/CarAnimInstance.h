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
    float DeltaRotation = 0.f;

    UPROPERTY(BlueprintReadWrite)
    float Steering = 0.f;
};

UCLASS()
class RAYCASTCAR_API UCarAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vehicle")
    TArray<FWheelAnimData> WheelData;
};
