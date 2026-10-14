// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WallRunAbility.generated.h"

#define ECC_Parkour ECC_GameTraceChannel1
#define ECC_WallRun ECC_GameTraceChannel2
/**
 * 
 */
UCLASS()
class TRAVERSAL_API UWallRunAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UWallRunAbility();

	/** Distance to Detect Wall **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float WallDetection = 50.f;

	/** How far character can run on wall **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float MaxWallRunDistance = 600.f;

	/** Character Vertical velocity while running on wall **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float WallRunVerticalVelocity = 100.f;

	/** Character Horizontal velocity while running on wall **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float WallRunHorizintalVelocity = 700.f;

	/** Maximum Z Height while runing horizontall **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float MinimumWllHeightToRun = 300.f;

	/** Maximum Vertical wallrun distance **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float MaxWallRunVerticalDistance = 600.f;

	/** Character Vertical (on Z axis) velocity while running on wall **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BalanceValue")
	float WallRunVerticallVelocity = 700.f;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateCancelAbility) override;

	void RunTaskHorizontal(bool IsLeftRun);
	void RunTaskVertical();
	void StartWallRun(bool IsLeftRun, FVector WallNormal);
	void StartVerticallWallRun(FVector WallNormal);

	UFUNCTION()
	void OnWallRunTaskFinished();
};
