// Copyright Natali Caggiano. All Rights Reserved.

#include "GameDataBridge.h"
#include "UnrealClaudeModule.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// Static WebSocket — one connection per game instance (PIE or packaged).
static TSharedPtr<IWebSocket> GGameSocket;

void UGameDataBridge::ConnectToPlugin(int32 Port)
{
	if (GGameSocket.IsValid() && GGameSocket->IsConnected())
	{
		UE_LOG(LogUnrealClaude, Log, TEXT("GameDataBridge: Already connected to plugin on port %d"), Port);
		return;
	}

	if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
	{
		FModuleManager::Get().LoadModule(TEXT("WebSockets"));
	}

	const FString Url = FString::Printf(TEXT("ws://localhost:%d"), Port);
	GGameSocket = FWebSocketsModule::Get().CreateWebSocket(Url, TEXT(""));

	GGameSocket->OnConnected().AddLambda([Port]()
	{
		UE_LOG(LogUnrealClaude, Log, TEXT("GameDataBridge: Connected to Claude plugin (port %d)"), Port);
	});

	GGameSocket->OnConnectionError().AddLambda([](const FString& Error)
	{
		UE_LOG(LogUnrealClaude, Warning, TEXT("GameDataBridge: Connection error — %s. Is the MCP bridge running?"), *Error);
	});

	GGameSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool /*bWasClean*/)
	{
		UE_LOG(LogUnrealClaude, Log, TEXT("GameDataBridge: Connection closed (%d) — %s"), StatusCode, *Reason);
	});

	GGameSocket->Connect();
	UE_LOG(LogUnrealClaude, Log, TEXT("GameDataBridge: Connecting to ws://localhost:%d"), Port);
}

void UGameDataBridge::DisconnectFromPlugin()
{
	if (GGameSocket.IsValid())
	{
		GGameSocket->Close();
		GGameSocket.Reset();
	}
}

void UGameDataBridge::SendGameData(const FString& JsonData)
{
	if (!GGameSocket.IsValid() || !GGameSocket->IsConnected())
	{
		UE_LOG(LogUnrealClaude, Verbose, TEXT("GameDataBridge: SendGameData called but not connected — dropping message"));
		return;
	}

	GGameSocket->Send(JsonData);
}

void UGameDataBridge::SendRogueliteState(
	int32 WaveNumber,
	float CurrentHP,
	float MaxHP,
	const TArray<FString>& ActivePerks,
	int32 DroneCount,
	float DamageMultiplier)
{
	if (!GGameSocket.IsValid() || !GGameSocket->IsConnected())
	{
		return;
	}

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetNumberField(TEXT("wave"), WaveNumber);
	Json->SetNumberField(TEXT("hp"), CurrentHP);
	Json->SetNumberField(TEXT("maxHp"), MaxHP);
	Json->SetNumberField(TEXT("droneCount"), DroneCount);
	Json->SetNumberField(TEXT("damageMultiplier"), DamageMultiplier);

	TArray<TSharedPtr<FJsonValue>> PerkValues;
	for (const FString& Perk : ActivePerks)
	{
		PerkValues.Add(MakeShared<FJsonValueString>(Perk));
	}
	Json->SetArrayField(TEXT("perks"), PerkValues);

	FString Output;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	GGameSocket->Send(Output);
}

bool UGameDataBridge::IsConnectedToPlugin()
{
	return GGameSocket.IsValid() && GGameSocket->IsConnected();
}
