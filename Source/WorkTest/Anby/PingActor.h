// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "PingActor.generated.h"

UCLASS()
class WORKTEST_API APingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APingActor();


	void PreviewStart();
	void PreviewEnd();
	UPROPERTY(EditAnywhere,Category=Ping)
	TSubclassOf<UUserWidget> PingWidgetClass;

	//void CreateSceneComponentPingWidget(UUserWidget* _widget);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* WidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PingMesh;

	void GetDistaneOwnerToWidget(float _distance);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 3.0f;
	
	UFUNCTION(Server,Reliable, Category=Ping)
	void Ping();

	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
private:
	

	bool bIsPingActive;
};
