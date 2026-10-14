// Fill out your copyright notice in the Description page of Project Settings.

#include "ParkourAbility.h"
#include "GameplayTagContainer.h"
#include <Traversal/TraversalCharacter.h>
#include "KismetTraceUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

UParkourAbility::UParkourAbility()
{
	//forbid spam Parkour Ability
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTag ParkourTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Parkour.Active"));
	ActivationOwnedTags.AddTag(ParkourTag);
	ActivationBlockedTags.AddTag(ParkourTag);
}

void UParkourAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("Parkour!!"));

	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	bool bIsWallThick = false;
	float MaxStepHeight = Character->GetCharStepHeight();
	float CapsuleHalfHeight = Character->GetCharCapsuleHalfHeight();

	FVector Start;
	FVector End;
	FVector WallNormal;	
	FVector WallLocation;
	FVector WallTopPoint2;
	FVector WallTopPoint;
	FVector WallHeight;

	FVector ForvardV;
	FCollisionQueryParams QueryParam;

	ForvardV = Character->GetCharForwardVector();
	Start = FVector(Character->GetActorLocation().X, Character->GetActorLocation().Y, Character->GetActorLocation().Z - CapsuleHalfHeight + MaxStepHeight);
	End = ForvardV * MaxWallDetection + Start;

	FHitResult HitResult;

	//find neerest obstacle  (1)
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Parkour, QueryParam);
	//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHit, HitResult, FColor::Red, FColor::Green, 10);

	//if hit parkour obsticle
	if (bHit)
	{		
		WallLocation = HitResult.Location;		
		WallNormal = HitResult.Normal;		
		End = WallNormal * -10.f + WallLocation; //10.f - just a small step back
		Start = FVector(End.X, End.Y, End.Z + DistanceForLineTraceToWall); // from top to bottom check
		
		//Check obstacle top point	(2)
		bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Parkour, QueryParam);
		//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHit, HitResult, FColor::Yellow, FColor::Green, 10);

		//	check if height is OK to climb or vault
		if (bHit)
		{
			WallTopPoint = HitResult.Location;
			WallHeight = WallTopPoint - WallLocation;

			//check if Wall Height is too height
			if (WallHeight.Z < MaxWallHeightToClimb)
			{
				End = WallNormal * -WallThicknessCheck + WallLocation; //the same as previous End point but further
				Start = FVector(End.X, End.Y, End.Z + DistanceForLineTraceToWall);

				//get neer wall top point2	(3)
				bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Parkour, QueryParam);
				//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHit, HitResult, FColor::Purple, FColor::Green, 10);

				if (bHit)
				{
					WallTopPoint2 = HitResult.Location;			
					if (((WallTopPoint.Z - WallTopPoint2.Z) < MaxHeightDifference)) //check if top of the wall is too whide 
						bIsWallThick = true;
				}
				else
				{
					bIsWallThick = false;
				}

				//mantle or vault
				if (WallHeight.Z > MantlingHeightPoint)
				{
					//mantle
					UE_LOG(LogTemp, Log, TEXT("mantle!!"));
					UseMantle(Character, WallTopPoint, WallTopPoint2, WallNormal, WallLocation);
				}
				else
				{
					//vault
					UE_LOG(LogTemp, Log, TEXT("vault!!"));
					UseVault(bIsWallThick, Character, WallNormal, WallTopPoint);
				}

			}
			else
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
				return;
			}

		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
			return;
		}


	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
}

void UParkourAbility::UseMantle(ATraversalCharacter* Character,FVector WallTopPoint,FVector WallTopPoint2, FVector WallNormal, FVector WallLocation)
{
	UE_LOG(LogTemp, Log, TEXT("mantle!!"));

	FCollisionQueryParams QueryParam;
	QueryParam.AddIgnoredActor(Character);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	bool bHit = false;
	FHitResult HitResult;
	float CapsuleHalfHeight = Character->GetCharCapsuleHalfHeight();
	//Check if top of the wall is blocked	(4)
	bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, WallTopPoint + FVector(0, 0, 2), (FVector(0, 0, 1) * CapsuleHalfHeight * 2) + WallTopPoint2, ObjectParams, QueryParam);
	//DrawDebugLineTraceSingle(GetWorld(), WallTopPoint+FVector(0,0,2), (FVector(0, 0, 1)*CapsuleHalfHeight*2)+WallTopPoint2, EDrawDebugTrace::Persistent, bHit, HitResult, FColor::White, FColor::Green, 10);

	if (bHit)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}

	

	FVector NewLocation;
	FVector XYLoc = UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(WallNormal)) * DistanceToWall + Character->GetActorLocation();

	//1.5m mantle or 1m
	if ((WallTopPoint.Z - WallLocation.Z) > MantleDifference)
	{
		Character->CancelWallRun();		
		Character->SetCharCollisionAndMovementMode(ECollisionEnabled::NoCollision, EMovementMode::MOVE_Flying);

		NewLocation = FVector(XYLoc.X, XYLoc.Y, WallTopPoint.Z - Character->GetCharCapsuleHalfHeight() + Parkour1_5m);
		Character->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		Character->GetMovementComponent()->Velocity = FVector(0, 0, 0);
		PlayMontage(ParkourMontageMantle_1_5m);
		UE_LOG(LogTemp, Log, TEXT("mantle 1.5m!!"));
	}
	else
	{
		/* in game whith jump - this is useles dogshit */
		/*
		NewLocation = FVector(XYLoc.X, XYLoc.Y, WallTopPoint.Z - Character->GetCharCapsuleHalfHeight() + Parkour1m);
		Character->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		PlayMontage(ParkourMontageMantle_1m);
		UE_LOG(LogTemp, Log, TEXT("mantle 1m!!"));
		*/
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}
}


void UParkourAbility::UseVault(bool IsWallThick, ATraversalCharacter* Character,FVector WallNormal,FVector WallTopPoint)
{
	if (!IsWallThick)
	{
		Character->CancelWallRun();		
		Character->SetCharCollisionAndMovementMode(ECollisionEnabled::NoCollision, EMovementMode::MOVE_Flying);

		FVector NewLocation;

		//change vault anim based on movespeed
		if (Character->GetCharSpeed() > ChangeVaultingSpeed)
		{
			NewLocation = Character->GetActorLocation() + (WallNormal * DistanceToWall);
			NewLocation = FVector(NewLocation.X, NewLocation.Y, WallTopPoint.Z + ParkourVault);
			Character->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			Character->GetMovementComponent()->Velocity = FVector(0, 0, 0);
			PlayMontage(ParkourMontageVaultFast);
			UE_LOG(LogTemp, Log, TEXT("fast vault!!"));
		}
		else
		{
			NewLocation = FVector(Character->GetActorLocation().X, Character->GetActorLocation().Y, WallTopPoint.Z + ParkourVault);
			Character->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			Character->GetMovementComponent()->Velocity = FVector(0, 0, 0);
			PlayMontage(ParkourMontageVaultNormal);
			UE_LOG(LogTemp, Log, TEXT("normal vault!!"));
		}
	}
	else
	{
		//Here need new animation
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}
}

void UParkourAbility::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (this)
	{
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	Task->OnCompleted.AddDynamic(this, &UParkourAbility::OnMontageDone);
	Task->OnCancelled.AddDynamic(this, &UParkourAbility::OnMontageInterupted);
	Task->OnInterrupted.AddDynamic(this, &UParkourAbility::OnMontageInterupted);
	Task->ReadyForActivation();
	UE_LOG(LogTemp, Log, TEXT("Montage start playing"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Wtf"));
	}
}

void UParkourAbility::OnMontageDone()
{
	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
		return;
	}
	Character->SetCharCollisionAndMovementMode(ECollisionEnabled::QueryAndPhysics, EMovementMode::MOVE_Walking);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	UE_LOG(LogTemp, Log, TEXT("Montage Done"));
}

void UParkourAbility::OnMontageInterupted()
{
	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
		return;
	}

	Character->SetCharCollisionAndMovementMode(ECollisionEnabled::QueryAndPhysics, EMovementMode::MOVE_Walking);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	UE_LOG(LogTemp, Log, TEXT("Montage Interupted"));
}