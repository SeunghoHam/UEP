// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CableComponent.h"
#include "Anby.h"
#include "GrappleMoveComponent.generated.h"

USTRUCT()
struct FGrappleMove
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Throtlle; // 가속 (앞 / 뒤)

	float SteeringThrow; // 조향 ( 좌 / 우)

	UPROPERTY()
	float DeltaTime;
	
	UPROPERTY()
	float Time;

	// 커스텀해보자잇
	UPROPERTY()
	FVector Direction;
	
	UPROPERTY()
	FVector TargetLocation;
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UGrappleMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrappleMoveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	FVector GrappleDirection; // Start -> GrappleTarget UnitVector
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void ServerFireGrapple(const FVector& Start, const FVector& End);

	UFUNCTION(BlueprintCallable)
	void FireGrapple();
	

	UPROPERTY(Editanywhere, BlueprintReadWrite, Category=Grapple)
	TSubclassOf<class AActor> GrapplePointObject;
	
	
	UPROPERTY(EditAnywhere, Category=Grapple)
	float MaxGrappleDistance = 2000.0f;

	UPROPERTY(EditAnywhere, Category=Grapple)
	FName StartSocketName = "LeftHandSocket"; // 스켈레탈에 따라 바꿔줘야함


	// 
	//UFUNCTION(BlueprintCallable, Category=ViewType)
	//void CallSpringArmStateChange(EViewType _type);
	
	// 우클릭 지속동안은 이 범위로 보이고 클릭 해제시 그 위치로 전달하기
	UFUNCTION()
	void StartPreview();
	UFUNCTION()
	void StopPreview();
	
	void CheckGrappleHitPoint();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	

	UFUNCTION(Server,Reliable,  Category= Grapple)
	void SpawnGrappleActor(const FVector& _location);

	UFUNCTION(Server,Reliable,  Category= Grapple)
	void DestroyGrappleActor();

	
	UFUNCTION(Server,Reliable, BlueprintCallable, Category=Grappple)
	void StartGrappleTo();
	
	UFUNCTION(BlueprintCallable, Category=Grappple)
	void StartGrapple();
	
	UFUNCTION(Server,Reliable, BlueprintCallable, Category=Grappple)
	void CancelGrapple();

	UPROPERTY(ReplicatedUsing = OnRep_GrappleLocation)
	FVector ReplicatedLocation; // 이 위치로 캐릭터 위치 변경?
	
	UFUNCTION()
	void OnRep_GrappleLocation();

	void SetGravity(bool _enable, float _amount = 1.f);
	
	AActor* GetGrappleActor();
private:
	UPROPERTY(Replicated)
	FVector GrappleTargetLocation; // 타겟지점

	UPROPERTY(Replicated)
	FVector GrappleCreatePoint; // 액터 생성 지점
	
	UPROPERTY(Replicated)
	bool bIsGrappling =  false;
	
	bool bCancelGrapple = false;
	float GrappleSpeed = 1000.0f;
	float ElapsedTime = 0.0f;
	UPROPERTY(Replicated)
	AActor* GrappleActor = nullptr;

	UPROPERTY(Replicated)
	bool bIsHooked = false;
	
	AAnby* Owner;
	APlayerController* PC;
	FTimerHandle GrapplePreviewHandle;
	float inRate =0.02f;
	void GrappleCreateAndSetting();


// --------------------------------------------------- 테스트

public:
	void SimulateMove(const FGrappleMove& _Move);
	FVector GetVelocity(){ return Velocity;}
	void SetVelocity(FVector _value){ Velocity = _value; }
	void SetThrottle(float _value){ Throttle = _value; }
	void SetSteeringThrow(float _value){ SteeringThrow = _value; }
	FGrappleMove GetLastMove() { return LastMove;}
private:
	FGrappleMove CreateMove(float DeltaTime);

	// 각종 저항 옵션 - 움직임 방식이 바뀌면 바꿔야할듯?
	FVector GetAirResistance();
	FVector GetRollingResistance();

	void ApplyRotation(float DeltaTime, float SteeringThrow);

	void UpdateLocationFromVelocity(float DeltaTime);

	// The mass of the car (kg).
	UPROPERTY(EditAnywhere)
	float Mass = 10; // Default = 1000

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;
	
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	// Higher means more drag.
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	// Higher means more rolling resistance.
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	FVector Velocity;
public:
	
	float Throttle;
	float SteeringThrow;

	FGrappleMove LastMove;
	
};
