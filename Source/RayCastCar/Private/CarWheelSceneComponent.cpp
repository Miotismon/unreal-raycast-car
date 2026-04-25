// Fill out your copyright notice in the Description page of Project Settings.


#include "CarWheelSceneComponent.h"
#include "CarPawn.h"

// Sets default values for this component's properties
UCarWheelSceneComponent::UCarWheelSceneComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UCarWheelSceneComponent::BeginPlay()
{
    Super::BeginPlay();

    InitWheelRefData();
}


// Called every frame
void UCarWheelSceneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UCarWheelSceneComponent::CalculateAndApplyForces(float DeltaTime)
{
    FHitResult HitResult;
    if (LineTraceToGround(HitResult))
    {
        FVector TotalForce = FVector(0.0);
        TotalForce += CalculateSuspensionForce(HitResult.Distance);
        TotalForce += CalculateGripForce(DeltaTime);
        if (IsDrive)
        {
            TotalForce += CalculateAccelerationForce(HitResult.ImpactNormal);
        }
        if (IsBrake || IsHandbrake)
        {
            TotalForce += CalculateBrakingForce(DeltaTime);
        }
        



        BodyMesh->AddForceAtLocation(TotalForce, GetSuspensionStart());

        MoveAndRotateWheel(DeltaTime, HitResult.Distance);
    }
    else
    {
        MoveAndRotateWheel(DeltaTime, SuspensionMaxDistance);
    }

}

void UCarWheelSceneComponent::InitWheelRefData()
{

    USkeletalMesh* Mesh = BodyMesh->GetSkeletalMeshAsset();
    if (!Mesh) return;

    const FReferenceSkeleton& RefSkel = Mesh->GetRefSkeleton();

    BoneIndex = RefSkel.FindBoneIndex(WheelBoneName);

    if (BoneIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bone not found: %s"), *WheelBoneName.ToString());
        return;
    }

    RefTransformCS = GetRefPoseComponentTransform(RefSkel, BoneIndex);
}

FTransform UCarWheelSceneComponent::GetRefPoseComponentTransform(const FReferenceSkeleton& RefSkel, int32 BoneIndex)
{
    const TArray<FTransform>& RefPose = RefSkel.GetRefBonePose();

    FTransform Result = RefPose[BoneIndex];

    int32 ParentIndex = RefSkel.GetParentIndex(BoneIndex);

    while (ParentIndex != INDEX_NONE)
    {
        Result = Result * RefPose[ParentIndex];
        ParentIndex = RefSkel.GetParentIndex(ParentIndex);
    }

    return Result;
}

FVector UCarWheelSceneComponent::GetRefWheelWorldLocation() const
{

    if (BoneIndex == INDEX_NONE)
        return FVector::ZeroVector;

    const FTransform& CompTransform = BodyMesh->GetComponentTransform();

    FTransform WorldTransform = RefTransformCS * CompTransform;

    return WorldTransform.GetLocation();
}

FVector UCarWheelSceneComponent::GetSuspensionStart() const
{
    return GetRefWheelWorldLocation() + FVector(0.0f, 0.0f, SuspensionTopOffset);
}

bool UCarWheelSceneComponent::LineTraceToGround(FHitResult& OutHit) const
{
    FVector Start = GetSuspensionStart();
    FVector End = Start + GetUpVector() * SuspensionMaxDistance * -1.0;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        ECC_Visibility,
        Params
    );
    
    // Debug draw
    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        bHit ? FColor::Green : FColor::Red
    );

    return bHit;
}

FVector UCarWheelSceneComponent::CalculateSuspensionForce(float LineTraceDistance) const
{
    FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetSuspensionStart());
    float ProjectedVelocityUp = LinearVelocityAtPoint.Dot(GetUpVector());
    float SpringOffset = RestDistance - LineTraceDistance;
    float SpringForce = SpringOffset * SpringStrength - ProjectedVelocityUp * SpringDamping;
    FVector SpringForceVec = SpringForce * GetUpVector();

    // Debug draw
    DrawDebugDirectionalArrow(
        GetWorld(),
        GetComponentTransform().TransformPosition(DebugDrawOffset),
        GetComponentTransform().TransformPosition(DebugDrawOffset) + SpringForceVec / 100.0,
        1.0,
        FColor::Blue
    );

    return SpringForceVec;
}

FVector UCarWheelSceneComponent::CalculateGripForce(float DeltaTime) const
{
    // Grip in Wheel Right Direction
    ACarPawn* BasePawn = Cast<ACarPawn>(GetOwner());

    FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetSuspensionStart());
    float ProjectedVelocityRight = LinearVelocityAtPoint.Dot(GetRightVector());
    
    float GripFactor = FMath::Abs(ProjectedVelocityRight / LinearVelocityAtPoint.Length());
    float AvailableGrip = BasePawn->GripCurve->GetFloatValue(GripFactor);
    
    // Reduce Grip if Handbrake is pressed
    if (IsHandbrake)
    {
        float InputHandbrake = BasePawn->InputHandbrake;
        if (InputHandbrake > 0.0)
        {
            AvailableGrip = 0.1;
        }
    }

    FVector GripForceVec = (-ProjectedVelocityRight * AvailableGrip / DeltaTime) * GetRightVector() * TireMass;

    // Rolling Friction
    float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector());
    if (FMath::Abs(ProjectedVelocityForward) > 0.0)
    {	
        float RollingFrictionForce = -1 * RollingFrictionFactor * FMath::Sign(ProjectedVelocityForward);
        FVector RollingFrictionVec = RollingFrictionForce * GetForwardVector();
        GripForceVec += RollingFrictionVec;
    }

    // Debug draw
    DrawDebugDirectionalArrow(
        GetWorld(),
        GetComponentTransform().TransformPosition(DebugDrawOffset),
        GetComponentTransform().TransformPosition(DebugDrawOffset) + GripForceVec / 100.0,
        1.0,
        FColor::Red
    );

    return GripForceVec;
}

FVector UCarWheelSceneComponent::CalculateAccelerationForce(FVector SurfaceNormal) const
{
    ACarPawn* BasePawn = Cast<ACarPawn>(GetOwner());
    float TopSpeed = BasePawn->TopSpeed;
    float MaxTorque = BasePawn->MaxTorque;
    float InputThrottle = BasePawn->InputThrottle;

    FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetSuspensionStart());
    float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector());

    float NormalizedCarSpeed = FMath::Abs(ProjectedVelocityForward) / TopSpeed;

    if (NormalizedCarSpeed >= 1.0)
    {
        return FVector(0.0);
    }

    float AvailablePower = BasePawn->PowerCurve->GetFloatValue(NormalizedCarSpeed) * MaxTorque;
    FVector AccelForceVec = AvailablePower * InputThrottle * GetForwardVector();
    FVector AccelForceVecProj = FVector::VectorPlaneProject(AccelForceVec, SurfaceNormal);

    // Debug draw
    DrawDebugDirectionalArrow(
        GetWorld(),
        GetComponentTransform().TransformPosition(DebugDrawOffset),
        GetComponentTransform().TransformPosition(DebugDrawOffset) + AccelForceVecProj / 100.0,
        1.0,
        FColor::Green
    );
     
    return AccelForceVecProj;
}

FVector UCarWheelSceneComponent::CalculateBrakingForce(float DeltaTime) const
{
    ACarPawn* BasePawn = Cast<ACarPawn>(GetOwner());
    float BrakingStrength = BasePawn->BrakingStrength;
    float InputBrake = BasePawn->InputBrake;
    float InputHandbrake = BasePawn->InputHandbrake;


    FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetSuspensionStart());
    float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector());

    // Compute desired change in velocity
    float DesiredDeltaV = 0.0;
    if (IsBrake)
    {
        DesiredDeltaV += -FMath::Sign(ProjectedVelocityForward) * BrakingStrength * InputBrake * DeltaTime;
    }

    if (IsHandbrake)
    {
        DesiredDeltaV += -FMath::Sign(ProjectedVelocityForward) * BrakingStrength * InputHandbrake * DeltaTime;
    }

    // Clamp DeltaV so we don't reverse
    DesiredDeltaV = FMath::Clamp(DesiredDeltaV, -FMath::Abs(ProjectedVelocityForward), FMath::Abs(ProjectedVelocityForward));

    // Convert DeltaV to force: F = DeltaV / DeltaT * Mass
    float BrakingForce = (DesiredDeltaV / DeltaTime) * TireMass;

    // make the Force based on the cars forward direction, cus braking with the wheels turned forces the car in to turn in the opposite direction.
    FVector BrakingForceVec = BrakingForce * BodyMesh->GetForwardVector();


    // Debug draw
    DrawDebugDirectionalArrow(
        GetWorld(),
        GetComponentTransform().TransformPosition(DebugDrawOffset),
        GetComponentTransform().TransformPosition(DebugDrawOffset) + BrakingForceVec / 100.0,
        1.0,
        FColor::Purple
    );


    /*if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1,
            0.0f,
            FColor::Yellow,
            FString::Printf(
                TEXT("InputBrake in Wheel: %f\n")
                TEXT("BrakingForceVec: %s"),
                InputBrake, *BrakingForceVec.ToString())
        );
    }*/
        


    return BrakingForceVec;
}

void UCarWheelSceneComponent::MoveAndRotateWheel(float DeltaTime, float SuspensionLength)
{
    // Move Mesh to where the floor is or the max length of the suspension
    float NewRelativeDistance = 0.0f;
    NewRelativeDistance = SuspensionLength - WheelMeshRadius;
    //MeshHolder->SetRelativeLocation(FVector(0.0, 0.0, -NewRelativeDistance));


    // Rotate Wheel
    FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetRefWheelWorldLocation());
    float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector()); // this is in cm/s
    float AngularSpeedDeg = FMath::RadiansToDegrees(ProjectedVelocityForward / WheelMeshRadius);
    float DeltaRotationDeg = -AngularSpeedDeg * DeltaTime;
    FRotator WheelRotation = FRotator(DeltaRotationDeg, 0.0, 0.0);
    //WheelMesh->AddLocalRotation(WheelRotation);
}

float UCarWheelSceneComponent::GetDeltaRotationDeg(float DeltaTime) const
{
    FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetRefWheelWorldLocation());
    float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector()); // this is in cm/s
    float AngularSpeedDeg = FMath::RadiansToDegrees(ProjectedVelocityForward / WheelMeshRadius);
    float DeltaRotationDeg = -AngularSpeedDeg * DeltaTime;
    return DeltaRotationDeg;
}



