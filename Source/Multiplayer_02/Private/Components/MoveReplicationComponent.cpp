// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MoveReplicationComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UMoveReplicationComponent::UMoveReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UMoveReplicationComponent::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent = GetOwner()->FindComponentByClass<UCarMovementComponent>();
}


// Called every frame
void UMoveReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ensureMsgf(MovementComponent, TEXT("MovementComponent is not found")))
	{
		return;
	}
	// Get last move to send with RPC and check it on the server side
	auto LastMove = MovementComponent->GetLastMove();

	auto ControlledPawn = Cast<APawn>(GetOwner());
	if (!ensureMsgf(ControlledPawn, TEXT("Inside the movement component Pawn is not found!")))
	{
		return;
	}
	// Update Listen-Client state on the server for simulation purposes
	if (GetOwnerRole() == ROLE_Authority && ControlledPawn->IsLocallyControlled())
	{
		AddMoveToTheQueue(LastMove);
		UpdateServerState(LastMove);
	}
	// 
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		/* Add move to the queue and send it to the server (!!!) where
		   it would be simulated as Server-side ("Canonical" simulation) code */
		AddMoveToTheQueue(LastMove);
		Server_SendMove(LastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		SimulatedClientTick(DeltaTime);
	}
}

void UMoveReplicationComponent::SimulatedClientTick(float ClientDeltaTime)
{
	ClientTimeSinceUpdate += ClientDeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (!ensureMsgf(MovementComponent, TEXT("SimulatedClientTick: MovementComponent is not found")))
	{
		return;
	}

	// Find Lerp ratio based on server update time
	auto LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	// Get aim location and rotation
	auto TargetLocation = ServerState.Transform.GetLocation();
	auto TargetRotation = ServerState.Transform.GetRotation();

	// Additional variables for hermits cubic interpolation
	float VelocityDerivativeTime = ClientTimeBetweenLastUpdates * 100;
	FVector StartVelocityDerivative = ClientStartVelocity * VelocityDerivativeTime;
	FVector TargetVelocityDerivative = ServerState.Velocity * VelocityDerivativeTime;

	// Interpolate the location
	auto NextLocation = FMath::CubicInterp(ClientStartTransform.GetLocation(), 
										   StartVelocityDerivative, 
										   TargetLocation, 
										   TargetVelocityDerivative, 
										   LerpRatio);
	// Set a new location
	GetOwner()->SetActorLocation(NextLocation);

	// Find an interpolated velocity derivative
	FVector VelocityDerivative = FMath::CubicInterpDerivative(ClientStartTransform.GetLocation(),
																	StartVelocityDerivative,
																	TargetLocation,
																	TargetVelocityDerivative,
																	LerpRatio);
	FVector NextVelocity = VelocityDerivative / VelocityDerivativeTime;
	// Set a new velocity
	MovementComponent->SetVelocity(NextVelocity);

	// Interpolate the rotation
	auto NextRotation = FQuat::Slerp(ClientStartTransform.GetRotation(), TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NextRotation);
}

void UMoveReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMoveReplicationComponent, ServerState);
}

void UMoveReplicationComponent::AddMoveToTheQueue(FCarPawnMove Move)
{
	UnacknowledgeMovesArray.Add(Move);
}

void UMoveReplicationComponent::RemoveStaleMoves(FCarPawnMove LastMove)
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

void UMoveReplicationComponent::UpdateServerState(FCarPawnMove LastMove)
{
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
	ServerState.LastMove = LastMove;
}

void UMoveReplicationComponent::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
		case ROLE_AutonomousProxy:
			AutonomousProxyOnRep_ServerState();
			break;
		case ROLE_SimulatedProxy:
			SimulatedProxyOnRep_ServerState();
			break;
		default:
			break;
	}
}

void UMoveReplicationComponent::AutonomousProxyOnRep_ServerState()
{
	if (!ensureMsgf(MovementComponent, TEXT("MovementComponent is not found")))
	{
		return;
	}
	// Replicate actors state on the client if on the server it was changed
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	// Update unacknowledged moves
	RemoveStaleMoves(ServerState.LastMove);
	// Reproduce all moves after receiving the server move state
	for (const FCarPawnMove& Move : UnacknowledgeMovesArray)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UMoveReplicationComponent::SimulatedProxyOnRep_ServerState()
{
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.0f;
	ClientStartTransform = GetOwner()->GetActorTransform();
}
void UMoveReplicationComponent::Server_SendMove_Implementation(FCarPawnMove Move)
{
	if (!ensureMsgf(MovementComponent, TEXT("MovementComponent is not found")))
	{
		return;
	}
	// Simulate move on the server ("Canonical simulation)
	MovementComponent->SimulateMove(Move);
	// Save the canonical state on the server
	UpdateServerState(Move);
}

bool UMoveReplicationComponent::Server_SendMove_Validate(FCarPawnMove Move)
{
	return true;// TODO check move
}

FString UMoveReplicationComponent::GetEnumRoleString(ENetRole LocalRole)
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