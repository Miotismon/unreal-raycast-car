// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "CarWheelSceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CarAnimInstance.h"

// Sets default values
ACarPawn::ACarPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    RootMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("SkeletalMesh"));
    RootComponent = RootMesh;
    RootMesh->SetSimulatePhysics(true);


    CarWheels.SetNum(4);
    CarWheels[0] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentFL"));
    CarWheels[1] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentFR"));
    CarWheels[2] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentBL"));
    CarWheels[3] = ObjectInitializer.CreateDefaultSubobject<UCarWheelSceneComponent>(this, TEXT("CarWheelSceneComponentBR"));
    for (int i = 0; i < CarWheels.Num(); ++i)
    {
        CarWheels[i]->SetupAttachment(RootMesh);
        CarWheels[i]->BodyMesh = RootMesh;
    }

    SpringArm = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootMesh);
    SpringArm->TargetArmLength = 1000.0f;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritRoll = false;
    SpringArm->AddRelativeRotation(FRotator(-20.0, 0.0, 0.0));

    Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);



}

void ACarPawn::OnThrottleInput(float Value)
{
    InputThrottle = Value;
}

void ACarPawn::OnBrakeInput(float Value)
{
    InputBrake = Value;
}

void ACarPawn::OnHandbrakeInput(bool bPressed)
{
    InputHandbrake = bPressed ? 1.0f : 0.0f;
}

void ACarPawn::OnSteerInput(float Value)
{
    InputSteering = Value;
}

void ACarPawn::OnCameraInput(FVector2D Value)
{
    InputCamera = Value;

    float CameraTargetPitch = FMath::Lerp(CameraBaseRotation.Pitch, 30.0f, Value.Y);
    float CameraTargetYaw = FMath::Lerp(CameraBaseRotation.Yaw, 180.0f, Value.X);

    CameraTargetRotation = FRotator(CameraTargetPitch, CameraTargetYaw, 0.0f);
}


// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
    Super::BeginPlay();

    // give the car wheels the default values
    for (int i = 0; i < CarWheels.Num(); ++i)
    {
        CarWheels[i]->SuspensionTopOffset = this->SuspensionTopOffset;
        CarWheels[i]->SuspensionMaxDistance = this->SuspensionMaxDistance;
        CarWheels[i]->RestDistance = this->RestDistance;
        CarWheels[i]->SpringStrength = this->SpringStrength;
        CarWheels[i]->SpringDamping = this->SpringDamping;
        CarWheels[i]->TireGripFactor = this->TireGripFactor;
        CarWheels[i]->RollingFrictionFactor = this->RollingFrictionFactor;
        CarWheels[i]->TireMass = this->TireMass;
    }
    
    // Get AnimInstance
    CarAnimInstance = Cast<UCarAnimInstance>(RootMesh->GetAnimInstance());

    // set the initial camera target rotation to the base rotation
    CameraTargetRotation = CameraBaseRotation;
    
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Steering Smoothing
    InterpolatedSteering = FMath::FInterpTo(InterpolatedSteering, InputSteering, DeltaTime, 5.0f);

    // Wheel Ticks
    for (int i = 0; i < CarWheels.Num(); ++i)
    {
        if (CarWheels[i]->IsSteering)
        {
            CarWheels[i]->SetRelativeRotation(FRotator(0.0, InterpolatedSteering * MaxSteeringAngle, 0.0));
        }

        CarWheels[i]->CalculateAndApplyForces(DeltaTime);

    }
    UpdateAnimWheelData(DeltaTime);

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
                TEXT("SmoothedSteering: %f\n")
                TEXT("STATS:\n")
                TEXT("Velocity: %s\n")
                TEXT(""), 
                InputThrottle, InputBrake, InputHandbrake, InputSteering, InterpolatedSteering, *RootMesh->GetPhysicsLinearVelocity().ToString() )
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
//void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//    Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//
//    APlayerController* PlayerController = Cast<APlayerController>(GetController());
//
//    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
//
//    Subsystem->ClearAllMappings();
//    Subsystem->AddMappingContext(CarMappingContext, 0);
//
//    UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
//
//    EIC->BindAction(IA_Throttle, ETriggerEvent::Triggered, this, &ACarPawn::OnThrottleInput);
//    EIC->BindAction(IA_Throttle, ETriggerEvent::Completed, this, &ACarPawn::OnThrottleInput);
//
//    EIC->BindAction(IA_Brake, ETriggerEvent::Triggered, this, &ACarPawn::OnBrakeInput);
//    EIC->BindAction(IA_Brake, ETriggerEvent::Completed, this, &ACarPawn::OnBrakeInput);
//    
//    EIC->BindAction(IA_Handbrake, ETriggerEvent::Triggered, this, &ACarPawn::OnHandbrakeInput);
//    EIC->BindAction(IA_Handbrake, ETriggerEvent::Completed, this, &ACarPawn::OnHandbrakeInput);
//
//    EIC->BindAction(IA_Steer, ETriggerEvent::Triggered, this, &ACarPawn::OnSteerInput);
//    EIC->BindAction(IA_Steer, ETriggerEvent::Completed, this, &ACarPawn::OnSteerInput);
//
//    EIC->BindAction(IA_Camera, ETriggerEvent::Triggered, this, &ACarPawn::OnCameraInput);
//    EIC->BindAction(IA_Camera, ETriggerEvent::Completed, this, &ACarPawn::OnCameraInput);
//}


void ACarPawn::UpdateAnimWheelData(float DeltaTime)
{   
    if (!CarAnimInstance) { 
        UE_LOG(LogTemp, Warning, TEXT("CarAnimInstance not found"));
        return; 
    }

    TArray<FWheelAnimData> NewWheelData;
    NewWheelData.SetNum(CarWheels.Num());

    for (int i = 0; i < CarWheels.Num(); ++i)
    {
        NewWheelData[i].WheelZOffset = CarWheels[i]->GetWheelZOffset();

        NewWheelData[i].RotationDeg = CarWheels[i]->GetWheelRotationDeg();
        
        if (CarWheels[i]->IsSteering)
        {
            NewWheelData[i].SteeringDeg = InterpolatedSteering * MaxSteeringAngle;
        }
        else
        {
            NewWheelData[i].SteeringDeg = 0.0f;
        }
    }

    CarAnimInstance->WheelData = NewWheelData;
}
