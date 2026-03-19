// Copyright Natali Caggiano. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameDataBridge.generated.h"

/**
 * Blueprint function library for streaming live game state to the UnrealClaude plugin.
 *
 * Usage (in Blueprint):
 *   BeginPlay  →  ConnectToPlugin()
 *   Event Tick / Wave End / Perk Applied  →  SendRogueliteState(...)
 *   EndPlay    →  DisconnectFromPlugin()
 *
 * The plugin's Node.js bridge receives the data on ws://localhost:8765 and
 * exposes it via the `unreal_live_game_data` MCP tool so Claude can query it.
 */
UCLASS()
class UNREALCLAUDE_API UGameDataBridge : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Connect to the UnrealClaude plugin WebSocket server.
	 * Call once at game start — e.g. in GameMode or PlayerController BeginPlay.
	 * Default port matches the bridge's GAME_WS_PORT (8765).
	 */
	UFUNCTION(BlueprintCallable, Category = "Claude|GameData",
		meta = (DisplayName = "Connect To Claude Plugin", AdvancedDisplay = "Port"))
	static void ConnectToPlugin(int32 Port = 8765);

	/**
	 * Disconnect from the plugin. Call from EndPlay or on quit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Claude|GameData",
		meta = (DisplayName = "Disconnect From Claude Plugin"))
	static void DisconnectFromPlugin();

	/**
	 * Send raw JSON to the plugin. Use this if you need full control over the payload.
	 * Example: SendGameData("{\"wave\":3,\"hp\":85}")
	 */
	UFUNCTION(BlueprintCallable, Category = "Claude|GameData",
		meta = (DisplayName = "Send Game Data (Raw JSON)"))
	static void SendGameData(const FString& JsonData);

	/**
	 * Convenience node — serializes roguelite game state and sends it.
	 * Matches the DT_ItemData / BP_PlayerCharacter / BP_GameState schema.
	 */
	UFUNCTION(BlueprintCallable, Category = "Claude|GameData",
		meta = (DisplayName = "Send Roguelite State", AdvancedDisplay = "DroneCount, DamageMultiplier"))
	static void SendRogueliteState(
		int32 WaveNumber,
		float CurrentHP,
		float MaxHP,
		const TArray<FString>& ActivePerks,
		int32 DroneCount = 0,
		float DamageMultiplier = 1.0f
	);

	/** Returns true if the WebSocket connection to the plugin is currently open. */
	UFUNCTION(BlueprintPure, Category = "Claude|GameData",
		meta = (DisplayName = "Is Connected To Claude Plugin"))
	static bool IsConnectedToPlugin();
};
