// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_02GameMode.h"
#include "Multiplayer_02Pawn.h"
#include "Multiplayer_02Hud.h"

AMultiplayer_02GameMode::AMultiplayer_02GameMode()
{
	DefaultPawnClass = AMultiplayer_02Pawn::StaticClass();
	HUDClass = AMultiplayer_02Hud::StaticClass();
}
