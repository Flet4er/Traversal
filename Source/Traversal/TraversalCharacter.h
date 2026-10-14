// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "MyAttributeSet.h"
#include "TraversalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ATraversalCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Sprint Input Action **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Parkour Input Mantle Action **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpecialAction;

	/** Fast Stance Action **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FastStanceAction;

	/** Fast Stance Action **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* W_Action;

public:
	ATraversalCharacter();
	
	/** Max character speed while sprinting **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharSprintSpeed = 800.f;

	/** Default character speed **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefCharSprintSpeed = 0.f;

	UFUNCTION()
	void CancelWallRun();

	UFUNCTION()
	void SetIsAfterWallJump(bool IsAfterWallJUmp);

	UFUNCTION()
	bool GetIsAfterWallJump();

	UFUNCTION()
	bool GetIsSprinting();

	UFUNCTION()
	void SetIsSprinting(bool Sprinting);

	UFUNCTION()
	float GetDefCharWalkSpeed();

	UFUNCTION()
	float GetCharStepHeight();

	UFUNCTION()
	float GetCharCapsuleHalfHeight();

	UFUNCTION()
	FVector GetCharForwardVector();

	UFUNCTION()
	float GetCharSpeed();
	UFUNCTION(BlueprintCallable)
	bool GetIsInFastStance();
	UFUNCTION()
	void SetIsInFastStance(bool isInFastStance);
	UFUNCTION(BlueprintCallable)
	bool GetIsOnWallRun();
	UFUNCTION(BlueprintCallable)
	bool GetIsOnLeftWallRun();
	UFUNCTION()
	void SetIsOnWallRun(bool isOnWallRun);
	UFUNCTION()
	void SetIsOnLeftWallRun(bool isOnVerticalWallRun);
	UFUNCTION(BlueprintCallable)
	bool GetIsOnVerticalWallRun();
	UFUNCTION()
	void SetIsOnVerticalWallRun(bool isOnVerticalWallRun);

	UFUNCTION()
	bool GetIsFalling();
	UFUNCTION()
	bool GetWKeyIsPressed();
	UFUNCTION()
	void SetWKeyIsPressed(bool IsPressed);

	UFUNCTION()
	void SetCharCollisionAndMovementMode(ECollisionEnabled::Type Collision, EMovementMode MoveMode);

	//GAS getter from interface
	UFUNCTION()
	UAbilitySystemComponent* GetAbilitySystemComponent() const;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	/** Called when use Jump key **/
	UFUNCTION()
	void JumpActivate(const FInputActionValue& Value);

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	/** Called when Holding Sprint key **/
	UFUNCTION()
	void SprintOngoing(const FInputActionValue& Value);

	/** Called when use Sprint key **/
	UFUNCTION()
	void SprintStarted(const FInputActionValue& Value);

	/** Called when Sprint key is canceled **/
	UFUNCTION()
	void SprintCanceled(const FInputActionValue& Value);

	/** Called when Sprint key is completed **/
	UFUNCTION()
	void SprintCompleted(const FInputActionValue& Value);

	/** Called when use FastStance key **/
	UFUNCTION()
	void FastStanceOngoing(const FInputActionValue& Value);
	/** Called when FastStance key is canceled **/
	UFUNCTION()
	void FastStanceCanceled(const FInputActionValue& Value);
	/** Called when FastStance key is completed **/
	UFUNCTION()
	void FastStanceCompleted(const FInputActionValue& Value);
	/** Called when FastStance key is trigered **/
	UFUNCTION()
	void FastStanceStarted(const FInputActionValue& Value);

	/** Called when use Parkour Mantle key **/
	UFUNCTION()
	void ParkourTriggered(const FInputActionValue& Value);

	/** W canceled **/
	UFUNCTION()
	void W_canceled(const FInputActionValue& Value);
	/** W completed **/
	UFUNCTION()
	void W_completed(const FInputActionValue& Value);
	/** W pressed **/
	UFUNCTION()
	void W_pressed(const FInputActionValue& Value);

	bool bIsSprinting = false;
	bool bIsInFastStance = false;
	bool bIsOnWallRun = false;
	bool bIsOnLeftWallRun = false;
	bool bIsOnVerticalWallRun = false;
	bool bIsW_KeyPressed = false;
	bool bIsAfterWallJump = false;

	//GAS Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameplayAbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	//AttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameplayAbilitySystem")
	UMyAttributeSet* AttributeSet;

	//Parkour Ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbilitySystem")
	TSubclassOf<UGameplayAbility> ParkourAbilityClass;

	//Parkour Ability Handle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbilitySystem")
	FGameplayAbilitySpecHandle ParkourAbilityHandle;

	//WallJump Ability Handle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbilitySystem")
	FGameplayAbilitySpecHandle WallJumpAbilityHandle;

	//WallRun Ability Handle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbilitySystem")
	FGameplayAbilitySpecHandle WallRunAbilityHandle;

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

