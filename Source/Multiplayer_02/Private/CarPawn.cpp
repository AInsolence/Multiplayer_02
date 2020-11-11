// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

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

void ACarPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACarPawn, ServerState);
}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();
	NetUpdateFrequency = 1.0f;
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Create and send move to the server
	if (IsLocallyControlled())
	{
		FCarPawnMove Move;
		Move.DeltaTime = DeltaTime;
		Move.SteeringThrow = SteeringThrow;
		Move.Throttle = Throttle;
		// TODO set TimeOfExecuting;

		if (!HasAuthority())
		{
			// Simulate move on the client
			SimulateMove(Move);
			//Add move to the unacknowledge moves array
			UnacknowledgeMovesArray.Add(Move);
			UE_LOG(LogTemp, Warning, TEXT("Moves queue length = %d"), UnacknowledgeMovesArray.Num());
		}
		// Send move to the server checking
		Server_SendMove(Move);
	}


	// Show Net Role
	DrawDebugString(GetWorld(),
		FVector(0, 0, 100),
		GetEnumRoleString(this->GetLocalRole()),
		this,
		FColor::White,
		DeltaTime);
}

void ACarPawn::SimulateMove(FCarPawnMove Move)
{
	// Driving
	UpdateLocationFormVelocity(Move.DeltaTime, Move.Throttle);
	// Steering
	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
}

void ACarPawn::RemoveStaleMoves(FCarPawnMove LastMove)
{
	TArray<FCarPawnMove> NewMoves;
	for (const FCarPawnMove& Move : UnacknowledgeMovesArray)
	{
		if (Move.TimeOfExecuting > LastMove.TimeOfExecuting)
		{
			NewMoves.Add(Move);
		}
	}
	UnacknowledgeMovesArray = NewMoves;
}

void ACarPawn::OnRep_ServerState()
{// Replicate actors state on the client if on the server it was changed
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
	// Update unacknowledge moves
	RemoveStaleMoves(ServerState.LastMove);
}

void ACarPawn::ApplyRotation(float DeltaTime, float SteeringThrowToSet)
{
	// Find turn angle in this frame
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float TurnAngle = DeltaLocation/SteeringRadius * SteeringThrowToSet;
	FQuat TurnRotation(GetActorUpVector(), TurnAngle);
	AddActorWorldRotation(TurnRotation);
	// Turn car velocity vector
	Velocity = TurnRotation.RotateVector(Velocity);
}

void ACarPawn::UpdateLocationFormVelocity(float DeltaTime, float ThrottleToSet)
{
	// Find driving force
	DrivingForce = GetActorForwardVector() * EnginePowerInNewtons * ThrottleToSet;
	// Find and apply air resistance and rolling resistance forces
	DrivingForce += GetAirResistance();
	DrivingForce += GetRollingResistance();
	// Find acceleration
	FVector Acceleration = DrivingForce / MassOfTheCarInKg;
	// Find velocity
	Velocity += Acceleration * DeltaTime;
	// Find distance traveled in this frame
	FVector DistansPerFrame = Velocity * DeltaTime * 100;
	// Change car position
	FHitResult HitResult;
	AddActorWorldOffset(DistansPerFrame, true, &HitResult);
	// Check physical blocking
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

FVector ACarPawn::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * FMath::Square(Velocity.Size()) * CarDragCoefficient;
}

FVector ACarPawn::GetRollingResistance()
{
	float AccelerationDueToGravity = - GetWorld()->GetGravityZ()/100 * MassOfTheCarInKg;
	FVector NormalForce = - Velocity.GetSafeNormal() * RollingResistanceCoefficient * AccelerationDueToGravity;
	return NormalForce;
}

FString ACarPawn::GetEnumRoleString(ENetRole LocalRole)
{
	switch (LocalRole)
	{
		case ROLE_None:
			return "None";
			break;
		case ROLE_SimulatedProxy:
			return "SimulatedProxy";
			break;
		case ROLE_AutonomousProxy:
			return "AutonomousProxy";
			break;
		case ROLE_Authority:
			return "Authority";
			break;
		case ROLE_MAX:
			return "MAX";
			break;
		default:
			return "ERROR";
			break;
	}
}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACarPawn::MoveForward(float Value)
{
	Throttle = Value;
}

void ACarPawn::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void ACarPawn::Server_SendMove_Implementation(FCarPawnMove Move)
{// Simulate move
	SimulateMove(Move);
	// Send the canonical state to the server
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
	ServerState.LastMove = Move;
}

bool ACarPawn::Server_SendMove_Validate(FCarPawnMove Move)
{
	return true;// TODO check move
}
