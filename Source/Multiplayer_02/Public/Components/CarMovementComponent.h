// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarMovementComponent.generated.h"

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYER_02_API UCarMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCarMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Move simulation helpers
	FCarPawnMove CreateMove(float DeltaTime);
	void SimulateMove(const FCarPawnMove& Move);

	// Driving
	void UpdateLocationFormVelocity(float DeltaTime, float ThrottleToSet);
	FORCEINLINE void SetVelocity(FVector VelocityToSet){ Velocity = VelocityToSet; }
	FORCEINLINE FVector GetVelocity() { return Velocity; }
	FORCEINLINE void SetThrottle(float ThrottleToSet) { Throttle = ThrottleToSet; }
	FORCEINLINE const FCarPawnMove GetLastMove() { return LastMove; }

	// Steering
	void ApplyRotation(float DeltaTime, float SteeringThrowToSet);
	FORCEINLINE void SetSteeringThrow(float SteeringThrowToSet) { SteeringThrow = SteeringThrowToSet; }

private:
	AActor* Owner = nullptr;
	FCarPawnMove LastMove;

	// Forces
	FVector GetAirResistance();
	FVector GetRollingResistance();

	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float CarDragCoefficient = 16.f;
	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float RollingResistanceCoefficient = 0.5f;


	// Driving
	float Throttle = 0.f;

	FVector DrivingForce;
	FVector Velocity;

	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float MassOfTheCarInKg = 1000;
	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float EnginePowerInNewtons = 10000;

	// Steering
	float SteeringThrow = 0.f;

	UPROPERTY(EditAnywhere, Category = "Car Properties")
	float SteeringRadius = 10.f;
};
