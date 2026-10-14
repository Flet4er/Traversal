// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunAbility.h"
#include <Traversal/TraversalCharacter.h>
#include "KismetTraceUtils.h"
#include "WallRunTask.h"
#include "WallRunVerticalTask.h"
#include "Components/CapsuleComponent.h"

UWallRunAbility::UWallRunAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTag WallRunTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.WallRun.Active"));
	ActivationOwnedTags.AddTag(WallRunTag);
	ActivationBlockedTags.AddTag(WallRunTag);
}

void UWallRunAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("WallRun!!"));

	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	FHitResult FHitRight;
	FHitResult FHitLeft;
	FHitResult FHitForward;
	FVector Start;
	FVector EndLeft;
	FVector EndRight;
	FVector End;
	FVector WallNormal;
	FCollisionQueryParams QueryParam;
	bool bHitRight = false;
	bool bHitLeft = false;
	bool bHitForward = false;

	Start = Character->GetActorLocation();
	EndRight = Character->GetActorRightVector() * WallDetection + Start;
	EndLeft = Character->GetActorRightVector() * WallDetection * -1 + Start;
	//RightTrace
	bHitRight = GetWorld()->LineTraceSingleByChannel(FHitRight, Start, EndRight, ECollisionChannel::ECC_WallRun, QueryParam);
	//DrawDebugLineTraceSingle(GetWorld(), Start, EndRight, EDrawDebugTrace::Persistent, bHitRight, FHitRight, FColor::Red, FColor::Green, 10);
	WallNormal = FHitRight.Normal;
	//LeftTrace
	bHitLeft = GetWorld()->LineTraceSingleByChannel(FHitLeft, Start, EndLeft, ECollisionChannel::ECC_WallRun, QueryParam);
	//DrawDebugLineTraceSingle(GetWorld(), Start, EndLeft, EDrawDebugTrace::Persistent, bHitLeft, FHitLeft, FColor::Red, FColor::Green, 10);

	/* detect wall to vertical run */
	//better remove this and left only botom check
	Start = FVector(Start.X, Start.Y, Start.Z + Character->GetCharCapsuleHalfHeight());
	End = Start + Character->GetActorForwardVector() * WallDetection;
	bHitForward = GetWorld()->LineTraceSingleByChannel(FHitForward, Start, End, ECollisionChannel::ECC_WallRun, QueryParam);
	//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHitForward, FHitForward, FColor::Red, FColor::Green, 10);

	if (bHitForward && !Character->GetIsFalling())
	{
		/* check if wall have botom */
		Start = FVector(Start.X, Start.Y, Start.Z - 2 * Character->GetCharCapsuleHalfHeight()+50);
		End = Start + Character->GetActorForwardVector() * WallDetection;
		bHitForward = GetWorld()->LineTraceSingleByChannel(FHitForward, Start, End, ECollisionChannel::ECC_WallRun, QueryParam);
		//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHitForward, FHitForward, FColor::Red, FColor::Green, 10);

		if (bHitForward)
		{
			/* check if wall have minimum height to run */
			Start = Character->GetActorLocation() + FVector(0, 0, MinimumWllHeightToRun);
			End = Start + Character->GetActorForwardVector() * WallDetection;
			bHitForward = GetWorld()->LineTraceSingleByChannel(FHitForward, Start, End, ECollisionChannel::ECC_WallRun, QueryParam);
			//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHitForward, FHitForward, FColor::Red, FColor::Green, 10);

			if (bHitForward)
			{
				WallNormal = FHitForward.Normal;

				StartVerticallWallRun(WallNormal);
			}

		}
	}
	else if(bHitRight && !Character->GetIsFalling())
	{	
		WallNormal = FHitRight.Normal;

		StartWallRun(false, WallNormal);
	}
	else if(bHitLeft && !Character->GetIsFalling())
	{	
		WallNormal = FHitLeft.Normal;

		StartWallRun(true, WallNormal);
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}	
}

void UWallRunAbility::CancelAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateCancelAbility)
{
	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Cancel WallRun!!"));
	Character->SetCharCollisionAndMovementMode(ECollisionEnabled::QueryAndPhysics, EMovementMode::MOVE_Falling);
	Character->SetIsOnWallRun(false);
	Character->SetIsOnVerticalWallRun(false);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UWallRunAbility::RunTaskHorizontal(bool IsLeftRun)
{	
	UWallRunTask* Task = UWallRunTask::NewAbilityTask<UWallRunTask>(this);
	Task->OnFinished.AddDynamic(this, &UWallRunAbility::OnWallRunTaskFinished);
	Task->MaxDistance = MaxWallRunDistance;
	Task->ClimbSpeed = WallRunVerticalVelocity;
	Task->RunSpeed = WallRunHorizintalVelocity;
	Task->bIsLeftRun = IsLeftRun;
	Task->WallDetection = WallDetection;
	Task->ReadyForActivation();
}

void UWallRunAbility::RunTaskVertical()
{
	UWallRunVerticalTask* Task = UWallRunVerticalTask::NewAbilityTask< UWallRunVerticalTask>(this);
	Task->OnFinished.AddDynamic(this, &UWallRunAbility::OnWallRunTaskFinished);
	Task->MaxDistance = MaxWallRunVerticalDistance;
	Task->RunSpeed = WallRunVerticallVelocity;
	Task->WallDetection = WallDetection;
	Task->ReadyForActivation();
}

void UWallRunAbility::OnWallRunTaskFinished()
{
	this->CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);
	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UWallRunAbility::StartWallRun(bool IsLeftRun, FVector WallNormal)
{
	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
		return;
	}

	Character->SetCharCollisionAndMovementMode(ECollisionEnabled::QueryAndPhysics, EMovementMode::MOVE_Flying);
	if (IsLeftRun)
	{
		FRotator Rotation = WallNormal.RotateAngleAxis(-90, FVector(0, 0, 1)).Rotation();
		Character->SetActorRotation(Rotation);
	}
	else
	{
		FRotator Rotation = WallNormal.RotateAngleAxis(90, FVector(0, 0, 1)).Rotation();
		Character->SetActorRotation(Rotation);
	}

	
	Character->SetIsOnWallRun(true);
	Character->SetIsOnLeftWallRun(IsLeftRun);
	RunTaskHorizontal(IsLeftRun);
}

void UWallRunAbility::StartVerticallWallRun(FVector WallNormal)
{
	ATraversalCharacter* Character = Cast<ATraversalCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
		return;
	}

	Character->SetCharCollisionAndMovementMode(ECollisionEnabled::QueryAndPhysics, EMovementMode::MOVE_Flying);
	FRotator Rotation = (WallNormal * -1).Rotation();
	Character->SetActorRotation(Rotation);

	Character->SetIsOnWallRun(true);
	Character->SetIsOnVerticalWallRun(true);
	RunTaskVertical();
}