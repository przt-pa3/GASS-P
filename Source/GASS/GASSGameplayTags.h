// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace GASSGameplayTags
{
	GASS_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// Declare all of the custom native tags that GASS will use
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	GASS_API	extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	GASS_API	extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);

	GASS_API	extern const TMap<FGameplayTag, uint8> CustomItemTagMap;
	GASS_API	extern const TMap<FGameplayTag, uint8> CustomEquipmentTagMap;

	// Item Tags
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Melee);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Ranged_Sub);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Ranged_Main);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Grenade);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Recovery);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Head);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Chest);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Hand);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Leg);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Armor_Feet);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Melee_Hammer);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Melee_Knife);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Ranged_Sub_Pistol);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Ranged_Main_Shotgun);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Equipment_Weapon_Ranged_Main_AssultRifle);

	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Bullet_Pistol);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Bullet_Shotgun);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Bullet_Rifle);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Gernade_Frag);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Gernade_Sound);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Gernade_Smoke);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Recovery_Health_Small);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Recovery_Health_Big);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Recovery_Shield_Small);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Recovery_Battery_Small);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Recovery_Battery_Big);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_ETC_Key);
	GASS_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Collectable);
};
