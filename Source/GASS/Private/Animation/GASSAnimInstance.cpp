// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/GASSAnimInstance.h"
#include "AbilitySystemGlobals.h"
#include "Character/GASSCharacter.h"
#include "Character/GASSCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSAnimInstance)


UGASSAnimInstance::UGASSAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGASSAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}

#if WITH_EDITOR
EDataValidationResult UGASSAnimInstance::IsDataValid(TArray<FText>& ValidationErrors)
{
	Super::IsDataValid(ValidationErrors);

	GameplayTagPropertyMap.IsDataValid(this, ValidationErrors);

	return ((ValidationErrors.Num() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif // WITH_EDITOR

void UGASSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UGASSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AGASSCharacter* Character = Cast<AGASSCharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}

	UGASSCharacterMovementComponent* CharMoveComp = CastChecked<UGASSCharacterMovementComponent>(Character->GetCharacterMovement());
	const FGASSCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
}

