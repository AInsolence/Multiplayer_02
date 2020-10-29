// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPlayerController.h"

void ACarPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ACarPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACarPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ACarPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACarPlayerController::MoveRight);
}

void ACarPlayerController::MoveForward(float Value)
{
	auto CarPawn = GetCarPawn();
	if (CarPawn)
	{
		CarPawn->MoveForward(Value);
	}
}

void ACarPlayerController::MoveRight(float Value)
{
	auto CarPawn = GetCarPawn();
	if (CarPawn)
	{
		CarPawn->MoveRight(Value);
	}
}

ACarPawn* ACarPlayerController::GetCarPawn()
{
	return Cast<ACarPawn>(GetPawn());
}