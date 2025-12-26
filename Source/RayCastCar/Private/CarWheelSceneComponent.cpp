// Fill out your copyright notice in the Description page of Project Settings.


#include "CarWheelSceneComponent.h"
#include "CarPawn.h"

// Sets default values for this component's properties
UCarWheelSceneComponent::UCarWheelSceneComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

TSet<TObjectPtr<USceneComponent>> UCarWheelSceneComponent::CreateChildComponents(const FObjectInitializer& ObjectInitializer, UObject* Parent)
{
	TSet<TObjectPtr<USceneComponent>> NewChildSet;
	
	MeshHolder = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(Parent, MakeUniqueObjectName(Parent, USceneComponent::StaticClass(), TEXT("MeshHolder")));
	CaliperMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(Parent, MakeUniqueObjectName(Parent, UStaticMeshComponent::StaticClass(), TEXT("CaliperMesh")));
	WheelMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(Parent, MakeUniqueObjectName(Parent, UStaticMeshComponent::StaticClass(), TEXT("WheelMesh")));
	
	NewChildSet.Add(MeshHolder);
	NewChildSet.Add(CaliperMesh);
	NewChildSet.Add(WheelMesh);
	return NewChildSet;
}


void UCarWheelSceneComponent::AttachChildComponents()
{
	MeshHolder->SetupAttachment(this);
	CaliperMesh->SetupAttachment(MeshHolder);
	WheelMesh->SetupAttachment(MeshHolder);
}

void UCarWheelSceneComponent::CalculateAndApplyForces(float DeltaTime)
{
	FHitResult HitResult;
	if (LineTraceToGround(HitResult))
	{
		FVector TotalForce;
		TotalForce += CalculateSuspensionForce(HitResult.Distance);
		TotalForce += CalculateGripForce(DeltaTime);
		if (IsDrive)
		{
			TotalForce += CalculateAccelerationForce(HitResult.ImpactNormal);
		}
		if (IsBrake)
		{
			TotalForce += CalculateBrakingForce(DeltaTime);
		}
		



		BodyMesh->AddForceAtLocation(TotalForce, GetComponentLocation());

		MoveAndRotateWheel(DeltaTime, HitResult.Distance);
	}
	else
	{
		MoveAndRotateWheel(DeltaTime, RayCastDistance);
	}

}

void UCarWheelSceneComponent::MoveAndRotateWheel(float DeltaTime, float SuspensionLength)
{
	// Move Mesh to where the floor is or the max length of the suspension
	float NewRelativeDistance = 0.0f;
	NewRelativeDistance = SuspensionLength - WheelMeshRadius;
	MeshHolder->SetRelativeLocation(FVector(0.0, 0.0, -NewRelativeDistance));


	// Rotate Wheel
	FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
	float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector()); // this is in cm/s
	float AngularSpeedDeg = FMath::RadiansToDegrees(ProjectedVelocityForward / WheelMeshRadius);
	float DeltaRotationDeg = -AngularSpeedDeg * DeltaTime;
	FRotator WheelRotation = FRotator(DeltaRotationDeg, 0.0, 0.0);
	WheelMesh->AddLocalRotation(WheelRotation);
}


bool UCarWheelSceneComponent::LineTraceToGround(FHitResult& OutHit)
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetUpVector() * RayCastDistance * -1.0;

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

FVector UCarWheelSceneComponent::CalculateSuspensionForce(float LineTraceDistance)
{
	FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
	float ProjectedVelocityUp = LinearVelocityAtPoint.Dot(GetUpVector());
	float SpringOffset = RestDistance - LineTraceDistance;
	float SpringForce = SpringOffset * SpringStrength - ProjectedVelocityUp * SpringDamping;
	FVector SpringForceVec = SpringForce * GetUpVector();

	// Debug draw
	DrawDebugDirectionalArrow(
		GetWorld(),
		GetComponentLocation(),
		GetComponentLocation() + SpringForceVec / 100.0,
		1.0,
		FColor::Blue
	);

	return SpringForceVec;
}

FVector UCarWheelSceneComponent::CalculateGripForce(float DeltaTime)
{
	//Grip in Wheel Right Direction
	FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
	float ProjectedVelocityRight = LinearVelocityAtPoint.Dot(GetRightVector());
	FVector GripForceVec = (-ProjectedVelocityRight * TireGripFactor / DeltaTime) * GetRightVector() * TireMass;

	//Rolling Friction
	float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector());
	if (FMath::Abs(ProjectedVelocityForward) != 0)
	{	
		float RollingFrictionForce = -1 * RollingFrictionFactor * FMath::Sign(ProjectedVelocityForward);
		FVector RollingFrictionVec = RollingFrictionForce * GetForwardVector();
		GripForceVec += RollingFrictionVec;
	}

	// Debug draw
	DrawDebugDirectionalArrow(
		GetWorld(),
		GetComponentLocation(),
		GetComponentLocation() + GripForceVec / 100.0,
		1.0,
		FColor::Red
	);

	return GripForceVec;
}

FVector UCarWheelSceneComponent::CalculateAccelerationForce(FVector SurfaceNormal)
{
	ACarPawn* BasePawn = Cast<ACarPawn>(GetOwner());
	float TopSpeed = BasePawn->TopSpeed;
	float MaxTorque = BasePawn->MaxTorque;
	float InputThrottle = BasePawn->InputThrottle;

	FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
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
		GetComponentLocation(),
		GetComponentLocation() + AccelForceVecProj / 100.0,
		1.0,
		FColor::Green
	);
	 
	return AccelForceVecProj;
}

FVector UCarWheelSceneComponent::CalculateBrakingForce(float DeltaTime)
{
	ACarPawn* BasePawn = Cast<ACarPawn>(GetOwner());
	float BrakingStrength = BasePawn->BrakingStrength;
	float InputBrake = BasePawn->InputBrake;


	FVector LinearVelocityAtPoint = BodyMesh->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
	float ProjectedVelocityForward = LinearVelocityAtPoint.Dot(GetForwardVector());

	//float BrakingForce = -1 * FMath::Sign(ProjectedVelocityForward) * BrakingStrength * InputBrake;
	//constexpr float StopSpeedEpsilon = 5.f; // cm/s
	//if (FMath::Abs(ProjectedVelocityForward) < StopSpeedEpsilon)
	//{
	//	return FVector(0.0); // do not apply braking
	//}
	//FVector BrakingForceVec = BrakingForce * GetForwardVector();

	// Compute desired change in velocity
	float DesiredDeltaV = -FMath::Sign(ProjectedVelocityForward) * BrakingStrength * InputBrake * DeltaTime;

	// Clamp DeltaV so we don't reverse
	DesiredDeltaV = FMath::Clamp(DesiredDeltaV, -FMath::Abs(ProjectedVelocityForward), FMath::Abs(ProjectedVelocityForward));

	// Convert DeltaV to force: F = DeltaV / DeltaT * Mass
	float BrakingForce = (DesiredDeltaV / DeltaTime) * TireMass;
	FVector BrakingForceVec = BrakingForce * GetForwardVector();


	// Debug draw
	DrawDebugDirectionalArrow(
		GetWorld(),
		GetComponentLocation(),
		GetComponentLocation() + BrakingForceVec / 100.0,
		1.0,
		FColor::Purple
	);


	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 
			0.0f, 
			FColor::Yellow, 
			FString::Printf(
				TEXT("InputBrake in Wheel: %f\n")
				TEXT("BrakingForceVec: %s"),
				InputBrake, *BrakingForceVec.ToString())
		);


	return BrakingForceVec;
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


