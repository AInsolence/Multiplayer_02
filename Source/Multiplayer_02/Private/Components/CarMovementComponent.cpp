// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CarMovementComponent.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UCarMovementComponent::UCarMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Owner = GetOwner();
	// ...
}


// Called when the game starts
void UCarMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UCarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	auto ControlledPawn = Cast<APawn>(Owner);
	if (!ensureMsgf(ControlledPawn, TEXT("Inside the movement component Pawn is not found!")))
	{
		return;
	}
	if (ControlledPawn->IsLocallyControlled())
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}

FCarPawnMove UCarMovementComponent::CreateMove(float DeltaTime)
{
	FCarPawnMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	if (ensureMsgf(Owner, TEXT("Movement component has no owner")))
	{
		if (GetWorld())
		{
			Move.TimeOfExecuting = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		}
	}
	
	return Move;
}

void UCarMovementComponent::SimulateMove(const FCarPawnMove& Move)
{
	// Driving
	UpdateLocationFormVelocity(Move.DeltaTime, Move.Throttle);
	// Steering
	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
}
void UCarMovementComponent::ApplyRotation(float DeltaTime, float SteeringThrowToSet)
{
	if (ensureMsgf(Owner, TEXT("Apply rotation does not work due to movement component has no owner")))
	{
		// Find turn angle in this frame
		float DeltaLocation = FVector::DotProduct(Owner->GetActorForwardVector(), Velocity) * DeltaTime;
		float TurnAngle = DeltaLocation / SteeringRadius * SteeringThrowToSet;
		FQuat TurnRotation(Owner->GetActorUpVector(), TurnAngle);
		Owner->AddActorWorldRotation(TurnRotation);
		// Turn car velocity vector
		Velocity = TurnRotation.RotateVector(Velocity);
	}
}

void UCarMovementComponent::UpdateLocationFormVelocity(float DeltaTime, float ThrottleToSet)
{
	if (ensureMsgf(Owner, TEXT("UpdateLocationFormVelocity does not work due to movement component has no owner")))
	{
		// Find driving force
		DrivingForce = Owner->GetActorForwardVector() * EnginePowerInNewtons * ThrottleToSet;
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
		Owner->AddActorWorldOffset(DistansPerFrame, true, &HitResult);
		// Check physical blocking
		if (HitResult.IsValidBlockingHit())
		{
			Velocity = FVector::ZeroVector;
		}
	}
}

FVector UCarMovementComponent::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * FMath::Square(Velocity.Size()) * CarDragCoefficient;
}

FVector UCarMovementComponent::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100 * MassOfTheCarInKg;
	FVector NormalForce = -Velocity.GetSafeNormal() * RollingResistanceCoefficient * AccelerationDueToGravity;
	return NormalForce;
}
