// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ParkourTypes.h"
#include "WallJumpAbility.generated.h"

#define ECC_Parkour ECC_GameTraceChannel1
/**
 * 
 */
UCLASS()
class TRAVERSAL_API UWallJumpAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UWallJumpAbility();

	/** Distance to wall detection **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float WallDetection = 50.f;

	/** Velocity of Jump Lenght. How further on XY character will be thrown **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float HorizontalVelocity = 1200.f;

	/** Velocity of Jump Height on Z **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float VerticalVelocity = 600.f;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	EWallRunDirection JumpDirection = EWallRunDirection::Forward;

	FRotator CharacterRotation;
};
