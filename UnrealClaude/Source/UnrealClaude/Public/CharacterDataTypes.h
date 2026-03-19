// Copyright Natali Caggiano. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "CharacterDataTypes.generated.h"

/**
 * Data Table row structure for character stats
 * Used with UDataTable for configurable character attributes
 */
USTRUCT(BlueprintType)
struct UNREALCLAUDE_API FCharacterStatsRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this stats configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName StatsId;

	/** Display name for this configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString DisplayName;

	/** Base health value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals", meta = (ClampMin = "0.0"))
	float BaseHealth = 100.0f;

	/** Maximum health value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals", meta = (ClampMin = "0.0"))
	float MaxHealth = 100.0f;

	/** Base stamina value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals", meta = (ClampMin = "0.0"))
	float BaseStamina = 100.0f;

	/** Maximum stamina value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals", meta = (ClampMin = "0.0"))
	float MaxStamina = 100.0f;

	/** Walking speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float WalkSpeed = 600.0f;

	/** Running/sprint speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float RunSpeed = 1000.0f;

	/** Jump height velocity in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float JumpVelocity = 420.0f;

	/** Damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float DamageMultiplier = 1.0f;

	/** Defense multiplier (damage reduction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float DefenseMultiplier = 1.0f;

	/** Experience points multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float XPMultiplier = 1.0f;

	/** Character level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", meta = (ClampMin = "1"))
	int32 Level = 1;

	/** Custom gameplay tags for this configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	TArray<FName> Tags;

	FCharacterStatsRow() = default;
};

/**
 * Data Asset for character configuration
 * Contains base stats, mesh/animation references, and gameplay settings
 */
UCLASS(BlueprintType)
class UNREALCLAUDE_API UCharacterConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Unique identifier for this character config */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ConfigId;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString DisplayName;

	/** Description of this character configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (MultiLine = "true"))
	FString Description;

	/** Reference to skeletal mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Reference to animation blueprint class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftClassPtr<UAnimInstance> AnimBlueprintClass;

	/** Walking speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseWalkSpeed = 600.0f;

	/** Running/sprint speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseRunSpeed = 1000.0f;

	/** Jump Z velocity in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseJumpVelocity = 420.0f;

	/** Max acceleration magnitude in cm/s² */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseAcceleration = 2048.0f;

	/** Ground friction coefficient (higher = stops faster) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseGroundFriction = 8.0f;

	/** Air control factor (0 = no control, 1 = full control while airborne) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseAirControl = 0.35f;

	/** Gravity scale multiplier (1.0 = normal gravity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseGravityScale = 1.0f;

	/** Starting health points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseHealth = 100.0f;

	/** Starting stamina points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseStamina = 100.0f;

	/** Base damage dealt per hit before multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseDamage = 10.0f;

	/** Flat damage reduction applied before damage multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseDefense = 0.0f;

	/** Capsule collision radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CapsuleRadius = 42.0f;

	/** Capsule collision half-height in cm (total height = 2 × this value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CapsuleHalfHeight = 96.0f;

	/** Optional reference to a stats DataTable for level-based progression */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TSoftObjectPtr<UDataTable> StatsTable;

	/** Default row name in stats table to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	FName DefaultStatsRowName;

	/** Gameplay tags for categorization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	TArray<FName> GameplayTags;

	/** Whether this is an NPC or player character config */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	bool bIsPlayerCharacter = false;

	/**
	 * Look up a stats row from the referenced DataTable.
	 * Falls back to DefaultStatsRowName when RowName is None.
	 * Returns a default-constructed row if the table is unset or the row is not found.
	 *
	 * @param RowName  Name of the row to fetch. Pass NAME_None to use DefaultStatsRowName.
	 * @return The matching FCharacterStatsRow, or a zeroed default if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Character Config")
	FCharacterStatsRow GetStatsRow(FName RowName = NAME_None) const
	{
		if (!StatsTable.IsNull())
		{
			if (UDataTable* Table = StatsTable.LoadSynchronous())
			{
				FName LookupName = RowName.IsNone() ? DefaultStatsRowName : RowName;
				if (FCharacterStatsRow* Row = Table->FindRow<FCharacterStatsRow>(LookupName, TEXT("")))
				{
					return *Row;
				}
			}
		}
		return FCharacterStatsRow();
	}
};
