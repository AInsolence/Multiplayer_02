// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "Multiplayer_02Hud.generated.h"


UCLASS(config = Game)
class AMultiplayer_02Hud : public AHUD
{
	GENERATED_BODY()

public:
	AMultiplayer_02Hud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
