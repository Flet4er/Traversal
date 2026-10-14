// Fill out your copyright notice in the Description page of Project Settings.


#include "WallJumpAbility.h"
#include <Traversal/TraversalCharacter.h>
#include "KismetTraceUtils.h"
#include "ParkourTypes.h"

UWallJumpAbility::UWallJumpAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTag WallJumpTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.WallJump.Active"));
	ActivationOwnedTags.AddTag(WallJumpTag);
	ActivationBlockedTags.AddTag(WallJumpTag);
	
	//settup trigers tag
	FAbilityTriggerData TriggerRight;
	TriggerRight.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.WallJump.Right"));
	TriggerRight.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	FAbilityTriggerData TriggerLeft;
	TriggerLeft.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.WallJump.Left"));
	TriggerLeft.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggers.Add(TriggerRight);
	AbilityTriggers.Add(TriggerLeft);

}

void UWallJumpAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (TriggerEventData)
	{
		UE_LOG(LogTemp, Log, TEXT("WallJumpTRIGGERS!!"));
	}
	UE_LOG(LogTemp, Log, TEXT("WallJump!!"));

	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	bool bHit = false;
	FHitResult FHit;
	FVector Start;
	FVector End;
	FCollisionQueryParams QueryParam;
	FVector WallNormal;

 	if (Character->GetIsFalling() || Character->GetIsOnWallRun())
	{
		if (TriggerEventData  && TriggerEventData->EventTag.IsValid())
		{
			FGameplayTag ReceivedTag = TriggerEventData->EventTag;
			//differs end position on Jump left\right\forward while on wallrun
			if (ReceivedTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.WallJump.Right"))))
			{
				End = Character->GetActorRightVector() * -1 * WallDetection + Character->GetActorLocation();
				CharacterRotation = Character->GetActorRotation() + FRotator(0, 90, 0);
			}
			else if (ReceivedTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.WallJump.Left"))))
			{
				End = Character->GetActorRightVector() * WallDetection + Character->GetActorLocation();
				CharacterRotation = Character->GetActorRotation() + FRotator(0, -90, 0);
			}
			else
			{
				End = Character->GetActorForwardVector() * WallDetection + Character->GetActorLocation();
				CharacterRotation = Character->GetActorRotation() + FRotator(0, 180, 0);
			}
		}
		else
		{
			//default walljump ** on vertical wallrun or regular walljump
			End = Character->GetActorForwardVector() * WallDetection + Character->GetActorLocation();
			CharacterRotation = Character->GetActorRotation() + FRotator(0, 180, 0);
		}

		Start = Character->GetActorLocation();

		bHit = GetWorld()->LineTraceSingleByChannel(FHit, Start, End, ECollisionChannel::ECC_Parkour, QueryParam);
		//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHit, FHit, FColor::Red, FColor::Green, 10);

		if (bHit)
		{
			Character->SetIsAfterWallJump(true);

			WallNormal = FHit.Normal;
			FVector LaunchVeloc = WallNormal * HorizontalVelocity + FVector(0, 0, VerticalVelocity);
			Character->LaunchCharacter(LaunchVeloc, true, true);
						
			Character->SetActorRotation(CharacterRotation);
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}