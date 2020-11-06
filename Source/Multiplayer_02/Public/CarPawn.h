// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CarPawn.generated.h"

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
	void Server_MoveForward(float Value);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Value);

private:

	// Replicate propertie by OnRep EVENT
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransform)
	FTransform ReplicatedTransform;
	UFUNCTION() // Replication function calls OnReps
	void OnRep_ReplicatedTransform();

	// Driving
	FVector DrivingForce;
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Car Net Properties")
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
