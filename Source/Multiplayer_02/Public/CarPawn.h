// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CarMovementComponent.h"
#include "Components/MoveReplicationComponent.h"
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
	class UBoxComponent* CollisionBox = nullptr;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	USkeletalMeshComponent* CarMesh = nullptr;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class USpringArmComponent* SpringArm = nullptr;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class UCameraComponent* Camera_01 = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class UMoveReplicationComponent* MoveReplicationComponent = nullptr;

public:	

	UPROPERTY(EditAnywhere, Category = "Car Components")
	class UCarMovementComponent* MovementComponent = nullptr;
	UPROPERTY(EditAnywhere, Category = "Car Components")
	class USceneComponent* MeshOffsetComponent = nullptr;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Input functions
	void MoveForward(float Value);
	void MoveRight(float Value);
};
