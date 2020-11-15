// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
ACarPawn::ACarPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Make acrtor be replicated
	bReplicates = true;

	// Create mesh component
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
	SetRootComponent(CollisionBox);
	// Create mesh component
	CarMesh = CreateDefaultSubobject<USkeletalMeshComponent>("CarMesh");
	// Create spring arm for the main camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	// Create main camera
	Camera_01 = CreateDefaultSubobject<UCameraComponent>("Camera_01");
	// Create a car movement component and set it replicated
	MovementComponent = CreateDefaultSubobject<UCarMovementComponent>("CarMovementComponent");
	// Create a car movement component and set it replicated
	MoveReplicationComponent = CreateDefaultSubobject<UMoveReplicationComponent>("MoveReplicationComponent");
	if (MoveReplicationComponent)
	{
		MoveReplicationComponent->SetIsReplicated(true);
	}

	// Set components hierarcy attachment
	if (ensureMsgf(CollisionBox, TEXT("CollisionBox component is not found")))
	{
		// Set physics
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

		if (ensureMsgf(CarMesh, TEXT("Mesh component is not found")))
		{
			CarMesh->SetupAttachment(CollisionBox);

			if (ensureMsgf(SpringArm, TEXT("SpringArm component is not found")))
			{
				SpringArm->TargetArmLength = 600.f;
				// Turn spring arm under the ground
				SpringArm->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, -25, 0)));
				SpringArm->SetupAttachment(CarMesh);

				if (ensureMsgf(Camera_01, TEXT("Camera_01 component is not found")))
				{
					Camera_01->FieldOfView = 90.f;
					Camera_01->SetupAttachment(SpringArm);
					Camera_01->Activate(true);
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		NetUpdateFrequency = 1.0f;
	}
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Show Net Role
	DrawDebugString(GetWorld(),
		FVector(0, 0, 100),
		MoveReplicationComponent->GetEnumRoleString(this->GetLocalRole()),
		this,
		FColor::White,
		DeltaTime);
}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACarPawn::MoveForward(float Value)
{
	if (!ensureMsgf(MovementComponent, TEXT("MovementComponent is not found")))
	{
		return;
	}
	MovementComponent->SetThrottle(Value);
}

void ACarPawn::MoveRight(float Value)
{
	if (!ensureMsgf(MovementComponent, TEXT("MovementComponent is not found")))
	{
		return;
	}
	MovementComponent->SetSteeringThrow(Value);
}
