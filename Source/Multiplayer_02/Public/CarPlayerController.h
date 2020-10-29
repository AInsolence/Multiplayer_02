// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CarPawn.h"
#include "CarPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYER_02_API ACarPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	/** Called for actor in a start of the game */
	virtual void BeginPlay() override;
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

protected:
	
	ACarPawn* GetCarPawn();

	virtual void SetupInputComponent() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
};
