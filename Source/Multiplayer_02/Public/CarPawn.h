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
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* CarMesh;
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera_01;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);

private:
	// Driving
	FVector DrivingForce;
	FVector Velocity;
	UPROPERTY(EditAnywhere)
	float MassOfTheCarInKg = 1000;
	UPROPERTY(EditAnywhere)
	float EnginePowerInNewtons = 10000;
	UPROPERTY(EditAnywhere)
	float CarDragCoefficient = 16.f;
	float Throttle = 0.f;

	void UpdateLocationFormVelocity(float DeltaTime);
	
	// Steering
	float SteeringThrow = 0.f;
	float MaxTurnAnglePerSecond = 90.f;

	void ApplyRotation(float DeltaTime);
};
