// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CarPawn.generated.h"

USTRUCT()
struct FCarPawnMove
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Car Move")
	float Throttle = 0.f;
	UPROPERTY(EditAnywhere, Category = "Car Move")
	float SteeringThrow = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Car Move")
	float DeltaTime;

	UPROPERTY(VisibleAnywhere, Category = "Car Move")
	float TimeOfExecuting;
};

USTRUCT()
struct FCarPawnState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Car State")
	FTransform Transform;
	UPROPERTY(EditAnywhere, Category = "Car State")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, Category = "Car State")
	FCarPawnMove LastMove;
};

UCLASS()
class MULTIPLAYER_02_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACarPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Components
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class UBoxComponent* CollisionBox;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	USkeletalMeshComponent* CarMesh;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class UCameraComponent* Camera_01;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Local functions
	void MoveForward(float Value);
	void MoveRight(float Value);

	// Server RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FCarPawnMove Move);

private:

	// Replicate property by OnRep EVENT
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FCarPawnState ServerState;
	UFUNCTION() // Replication function calls OnReps
	void OnRep_ServerState();

	// Driving
	FVector DrivingForce;
	FVector Velocity;
	
	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float MassOfTheCarInKg = 1000;
	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float EnginePowerInNewtons = 10000;
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Car Net Properties")
	float Throttle = 0.f;

	void UpdateLocationFormVelocity(float DeltaTime);
	
	// Steering
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Car Net Properties")
	float SteeringThrow = 0.f;

	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float SteeringRadius = 10.f;

	void ApplyRotation(float DeltaTime);

	// Forces
	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float CarDragCoefficient = 16.f;
	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float RollingResistanceCoefficient = 1.f;
	FVector GetAirResistance();
	FVector GetRollingResistance();
	//
	FString GetEnumRoleString(ENetRole Role);
};
