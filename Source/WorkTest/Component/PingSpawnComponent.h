// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PingSpawnComponent.generated.h"

class UUserWidget;
class APingActor;
class AAnby;
class AAnbyController;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UPingSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPingSpawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void PreviewStart();
	void PreviewEnd();

	void PreviewShow();
	bool bIsShowPreview = false;
	//void Ping(); // 실질적으로 핑 생성은 애가함

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Ping)
	FRotator WidgetRot;
	
	UFUNCTION(Server,Reliable, BlueprintCallable, Category=Ping)
	void ServerSendPing(const FVector& Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnPing(const FVector& Location);

	UPROPERTY(Replicated)
	FVector PingLocation;
	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<APingActor> PingActorClass;

	UPROPERTY()
	APingActor* PingActorInstance = nullptr;

	UFUNCTION()
	void PingChooseStart();
	//void PingChoose();
	void PingChooseEnd();

	void PrevDisable();
	void DisablePing();
	//bool isPreview = false;


	UFUNCTION()
	void GetDirection(float _yaw, float _pitch);
	
private:
	// 핑 선택하기
	FVector2D MouseStartPos;
	FVector2D MouseCurrentPos;
	FVector2D MouseEndPos;
	
	bool bIsCheckMouseDirection =false;
	
	class UPingWidget* PingWidget;
	//void LookAtPlayer();
	void SpawnPing();
	AAnby* anby;
	APlayerController* Controller;

	FVector SpawnLocation;


	// Direction
	float YawValue;
	float PitchValue;
	
	FTimerHandle PreviewHandle;
	FTimerHandle MouseDirectionTimerHandle;
	float inRate = 0.02f;
};
