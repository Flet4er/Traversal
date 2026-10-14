// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include <Traversal/TraversalCharacter.h>
#include "ParkourAbility.generated.h"

#define ECC_Parkour ECC_GameTraceChannel1
/**
 * 
 */
UCLASS()
class TRAVERSAL_API UParkourAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UParkourAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;


	/** Montage Mantle 1.5m **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ParkourMontageMantle_1_5m;

	/** Montage Mantle 1m **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ParkourMontageMantle_1m;

	/** Montage Vault fast **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ParkourMontageVaultFast;

	/** Montage Vault normal **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ParkourMontageVaultNormal;

	/** Z offset of character when teleporting using mantle 1.5m **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourBalanc")
	float Parkour1_5m = 30.f;

	/** Z offset of character when teleporting using mantle 1m**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourBalanc")
	float Parkour1m = 82.f;

	/** Z offset of character when teleporting using vaulting **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourBalanc")
	float ParkourVault = -10.f;

	/** distane to wall when teleporting character when parkour **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourBalanc")
	float DistanceToWall = 20.f;

	/** distane to wall when teleporting character when parkour **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourBalanc")
	float DistanceForLineTraceToWall = 250.f;

	/** Maximum Height to Forbid Climb **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float MaxWallHeightToClimb = 200.f;

	/** Height when mantling become vaulting **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float MantlingHeightPoint = 60.f;

	/** Distance of first raycast to wall. Wall Detection **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float MaxWallDetection = 85.f;

	/** Max wall thickness to forbid vaulting **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float WallThicknessCheck = 60.f;

	/** Difference between two top point on wall. If < then wall is thick **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float MaxHeightDifference = 20.f;

	/** Character Speed when normal animation of vaulting changing on fast animation **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float ChangeVaultingSpeed = 700.f;

	/** height when mantling 1m become 1.5m **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkourConst")
	float MantleDifference = 100.f;

	UFUNCTION()
	void OnMontageDone();
	UFUNCTION()
	void OnMontageInterupted();
	UFUNCTION()
	void PlayMontage(UAnimMontage* MontageToPlay);
	UFUNCTION()
	void UseVault(bool IsWallThick, ATraversalCharacter* Character, FVector WallNormal, FVector WallTopPoint);
	UFUNCTION()
	void UseMantle(ATraversalCharacter* Character, FVector WallTopPoint, FVector WallTopPoint2, FVector WallNormal, FVector WallLocation);
};
