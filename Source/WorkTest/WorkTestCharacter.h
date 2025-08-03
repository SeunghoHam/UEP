// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WorkTestCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/*
USTRUCT()
struct FActorMove
{
	// ������ OnTick�Ǿ���� ��ҵ�
	// �̵������� Throttle, Streeing
	// Deltatime�� OnTick(��������)���� ����Ұ�

	// ������ ���۽�ų ����(Thrttle, StreeingThrow)�� struct ���ο��� ������
	// ���� ������ �ʿ������ UPROPERTY ���𿡼� Replicated�� ��

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float DeltaTime;
};

USTRUCT()
struct FActorState
{
	// ���⼭ ���¸� Ȯ���Ұ�
	
	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FVector Transform;

	UPROPERTY()
	FActorMove LastMoves;
	// �������� ������ �̵��� �����Ҷ�
	// ���ε��� ���� �̵� ����� Ȯ���ؼ� �˻���
	// ������ �̵��� �����ϰ� ���ο� �̵����� ��ü�Ͽ� ����

};
*/
DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AWorkTestCharacter : public ACharacter
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* QskillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EskillAction;


	void Tick(float Deltatime) override;
public:
	AWorkTestCharacter();
	
	void QSkill();
	void ESkill();
	
protected:

	void TestJump();

	UFUNCTION(Server, Reliable) // WithValidation �߰��ϸ� _Validation �Լ��� �߰��� �������� bool Ÿ��
	void Server_Jump();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
private:
	// Replicated
	// 1. Uproperty(Replicated)�� ���� ����
	// 2. GetLifetimeReplicatedProps()�Լ� �ϼ�
	// 3. �����ϱ�

	// 
	//UPROPERTY(Replicated) // Replicated <-> ReplicatedUsing
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransform) // �μ�, ��ȯ ������ �ȵ�. ���� Ʈ������)
	FVector ReplicatedTransform;

	UPROPERTY(Replicated)
	FVector CustomVelocity;
	

	/*
	UPROPERTY(Replicated)
	FActorState ServerState;
	*/
	


	UFUNCTION()
	void OnRep_ReplicatedTransform(); // �μ�, ��ȯ ������ �ȵ�. ���� Ʈ������

	// Override Replicate Properties function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

