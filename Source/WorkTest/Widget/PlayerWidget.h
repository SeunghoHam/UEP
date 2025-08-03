// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

/**
 * 
 */
class UBossHPWidget;
UCLASS()
class WORKTEST_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	bool bIsUICenter =false;
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=UI)
	void AnimateGrappleCrosshair();
	
	virtual bool Initialize() override;

	UPROPERTY(meta=(BindWidget))
	UBossHPWidget* BossHPWidget;
	/*
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BossName;

	UPROPERTY(meta = (BindWidget))
	class UImage* CrossHair_Left;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* CrossHair_Right;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* CrossHair_Center;
	*/
};
