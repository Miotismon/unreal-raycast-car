// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "CarWheelSceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

// Sets default values
ACarPawn::ACarPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BodyMesh"));
	RootComponent = RootMesh;
	RootMesh->SetSimulatePhysics(true);


	CarWheels.SetNum(4);
	CarWheels[0] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentFL"));
	CarWheels[1] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentFR"));
	CarWheels[2] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentRL"));
	CarWheels[3] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentRR"));
	for (int i = 0; i < CarWheels.Num(); ++i)
	{
		CarWheels[i]->SetupAttachment(RootMesh);
		GrandchildComponents.Append(CarWheels[i]->CreateChildComponents(ObjectInitializer, this));
		CarWheels[i]->AttachChildComponents();
		CarWheels[i]->BodyMesh = RootMesh;
	}
	
	PopupLights = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("PopupLights"));
	PopupLights->SetupAttachment(RootMesh);

	SpringArm = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootMesh);
	SpringArm->TargetArmLength = 1000.0f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->AddRelativeRotation(FRotator(-20.0, 0.0, 0.0));

	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);



}

void ACarPawn::OnThrottleInput(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() == ETriggerEvent::Completed)
	{
		InputThrottle = 0.0f;
		return;
	}

	InputThrottle = Instance.GetValue().Get<float>();
	return;
}

void ACarPawn::OnBrakeInput(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() == ETriggerEvent::Completed)
	{
		InputBrake = 0.0f;
		return;
	}

	InputBrake = Instance.GetValue().Get<float>();
	return;
}

void ACarPawn::OnHandbrakeInput(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() == ETriggerEvent::Completed)
	{
		InputHandbrake = 0.0f;
		return;
	}

	InputHandbrake = Instance.GetValue().Get<float>();
	return;
}

void ACarPawn::OnSteerInput(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() == ETriggerEvent::Completed)
	{
		InputSteering = 0.0f;
		return;
	}

	InputSteering = Instance.GetValue().Get<float>();
	return;
}

void ACarPawn::OnCameraInput(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() == ETriggerEvent::Completed)
	{
		InputCamera = FVector2D(0.0);
		
	}
	else
	{
		InputCamera = Instance.GetValue().Get<FVector2D>();
	}

	float CameraTargetPitch = FMath::Lerp(CameraBaseRotation.Pitch, 30.0, InputCamera.Y);
	float CameraTargetYaw = FMath::Lerp(CameraBaseRotation.Yaw, 180.0, InputCamera.X);

	CameraTargetRotation = FRotator(CameraTargetPitch, CameraTargetYaw, 0.0);
}


// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();

	// give the car wheels the default values
	for (int i = 0; i < CarWheels.Num(); ++i)
	{
		CarWheels[i]->RayCastDistance = this->RayCastDistance;
		CarWheels[i]->RestDistance = this->RestDistance;
		CarWheels[i]->SpringStrength = this->SpringStrength;
		CarWheels[i]->SpringDamping = this->SpringDamping;
		CarWheels[i]->TireGripFactor = this->TireGripFactor;
		CarWheels[i]->RollingFrictionFactor = this->RollingFrictionFactor;
		CarWheels[i]->TireMass = this->TireMass;
	}
	
	// set the initial target rotation to the base rotation
	CameraTargetRotation = CameraBaseRotation;
	
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Wheel Calculations
	for (int i = 0; i < CarWheels.Num(); ++i)
	{
		if (CarWheels[i]->IsSteering)
		{
			CarWheels[i]->SetRelativeRotation(FRotator(0.0, InputSteering * MaxSteeringAngle, 0.0));
		}

		CarWheels[i]->CalculateAndApplyForces(DeltaTime);
	}

	// Camera Smoothing
	float CameraLerpAmount = 1.0 - FMath::Pow(CameraSmoothingRate, DeltaTime);
	FRotator NewCameraRotation = FMath::Lerp(SpringArm->GetRelativeRotation(), CameraTargetRotation, CameraLerpAmount);
	SpringArm->SetRelativeRotation(NewCameraRotation);



	// Debug Draw Velocity
	DrawDebugDirectionalArrow(
		GetWorld(),
		RootMesh->GetComponentLocation() + FVector(0.0, 0.0, 80.0),
		RootMesh->GetComponentLocation() + FVector(0.0, 0.0, 80.0) + RootMesh->GetPhysicsLinearVelocity() / 2.0,
		1.0,
		FColor::Yellow
	);
	// Debug Text
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			0.f,
			FColor::Orange,
			FString::Printf(
				TEXT("INPUTS:\n")
				TEXT("Throttle: %f\n")
				TEXT("Brake: %f\n")
				TEXT("Handbrake: %f\n")
				TEXT("Steering: %f\n")
				TEXT("STATS:\n")
				TEXT("Velocity: %s\n")
				TEXT(""), 
				InputThrottle, InputBrake, InputHandbrake, InputSteering, *RootMesh->GetPhysicsLinearVelocity().ToString() )
		);
	}
		
	

}

//void ACarPawn::PossessedBy(AController* NewController)
//{
//	Super::PossessedBy(NewController);
//
//	// setup Mapping Context in possessing player controller (TODO: make a custom player controller and do this in OnPossess())
//	if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
//	{
//		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
//		{
//			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
//			{
//				Subsystem->AddMappingContext(CarMappingContext, 0);
//			}
//		}
//	}
//}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(CarMappingContext, 0);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EIC->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &ACarPawn::OnThrottleInput);
	EIC->BindAction(IA_Throttle, ETriggerEvent::Completed, this, &ACarPawn::OnThrottleInput);

	EIC->BindAction(IA_Brake, ETriggerEvent::Triggered, this, &ACarPawn::OnBrakeInput);
	EIC->BindAction(IA_Brake, ETriggerEvent::Completed, this, &ACarPawn::OnBrakeInput);
	
	EIC->BindAction(IA_Handbrake, ETriggerEvent::Triggered, this, &ACarPawn::OnHandbrakeInput);
	EIC->BindAction(IA_Handbrake, ETriggerEvent::Completed, this, &ACarPawn::OnHandbrakeInput);

	EIC->BindAction(IA_Steer, ETriggerEvent::Triggered, this, &ACarPawn::OnSteerInput);
	EIC->BindAction(IA_Steer, ETriggerEvent::Completed, this, &ACarPawn::OnSteerInput);

	EIC->BindAction(IA_Camera, ETriggerEvent::Triggered, this, &ACarPawn::OnCameraInput);
	EIC->BindAction(IA_Camera, ETriggerEvent::Completed, this, &ACarPawn::OnCameraInput);
}
