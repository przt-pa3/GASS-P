// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASSGameplayTags.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "GASSLogChannels.h"

namespace GASSGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun, "InputTag.AutoRun", "Auto-run input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");

	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

	// When extending GASS, you can create your own movement modes but you need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See GASSGameplayTags::CustomMovementModeTagMap.");

	// Cutsom Item Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Bullet_Pistol, "Item.Consumable.Bullet.Pistol", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Bullet_Shotgun, "Item.Consumable.Bullet.Shotgun", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Bullet_Rifle, "Item.Consumable.Bullet.Rifle", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Gernade_Frag, "Item.Consumable.Gernade.Frag", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Gernade_Sound, "Item.Consumable.Gernade.Sound", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Gernade_Smoke, "Item.Consumable.Gernade.Smoke", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Recovery_Health_Small, "Item.Consumable.Recovery.Health.Small", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Recovery_Health_Big, "Item.Consumable.Recovery.Health.Big", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Recovery_Shield_Small, "Item.Consumable.Recovery.Shield.Small", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Recovery_Battery_Small, "Item.Consumable.Recovery.Battery.Small", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Recovery_Battery_Big, "Item.Consumable.Recovery.Battery.Big", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_ETC_Key, "Item.Consumable.ETC.Key", "Custom Item Tag");

	// Main Equipment Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Melee, "Item.Equipment.Weapon.Melee", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Melee_Hammer, "Item.Equipment.Weapon.Melee.Hammer", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Melee_Knife, "Item.Equipment.Weapon.Melee.Knife", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Ranged_Sub, "Item.Equipment.Weapon.Ranged.Sub", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Ranged_Sub_Pistol, "Item.Equipment.Weapon.Ranged.Sub.Pistol", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Ranged_Main, "Item.Equipment.Weapon.Ranged.Main", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Ranged_Main_Shotgun, "Item.Equipment.Weapon.Ranged.Main.Shotgun", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Weapon_Ranged_Main_AssultRifle, "Item.Equipment.Weapon.Ranged.Main.AssultRifle", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Grenade, "Item.Consumable.Grenade", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Consumable_Recovery, "Item.Consumable.Recovery", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Armor_Head, "Item.Equipment.Armor.Head", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Armor_Chest, "Item.Equipment.Armor.Chest", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Armor_Hand, "Item.Equipment.Armor.Hand", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Armor_Leg, "Item.Equipment.Armor.Leg", "Custom Item Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Equipment_Armor_Feet, "Item.Equipment.Armor.Feet", "Custom Item Tag");

	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with your custom modes
	};

	const TMap<FGameplayTag, uint8> CustomItemTagMap =
	{
		{	Item_Consumable_Bullet_Pistol			,	0	}	,
		{	Item_Consumable_Bullet_Shotgun			,	1	}	,
		{	Item_Consumable_Bullet_Rifle			,	2	}	,
		{	Item_Consumable_Gernade_Frag			,	3	}	,
		{	Item_Consumable_Gernade_Sound			,	4	}	,
		{	Item_Consumable_Gernade_Smoke			,	5	}	,
		{	Item_Consumable_Recovery_Health_Small	,	6	}	,
		{	Item_Consumable_Recovery_Health_Big		,	7	}	,
		{	Item_Consumable_Recovery_Shield_Small	,	8	}	,
		{	Item_Consumable_Recovery_Battery_Small	,	9	}	,
		{	Item_Consumable_Recovery_Battery_Big	,	10	}	,
		{	Item_Consumable_ETC_Key					,	11	}
	};

	const TMap<FGameplayTag, uint8> CustomEquipmentTagMap =
	{
		{	Item_Equipment_Armor_Head			,	0	}	,
		{	Item_Equipment_Armor_Chest			,	1	}	,
		{	Item_Equipment_Armor_Leg			,	2	}	,
		{	Item_Equipment_Armor_Hand			,	3	}	,
		{	Item_Equipment_Armor_Feet			,	4	}	,
		{	Item_Equipment_Weapon_Melee			,	5	}	,
		{	Item_Equipment_Weapon_Ranged_Sub	,	6	}	,
		{	Item_Equipment_Weapon_Ranged_Main	,	7	}	,
		{	Item_Consumable_Grenade				,	8	}	,
		{	Item_Consumable_Recovery				,	9	}	,
	};

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogGASS, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}

