// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PingWidget.h"

#include "Factories.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/WidgetSwitcher.h"
//#include "Engine/PlatformInterfaceBase.h"

bool UPingWidget::Initialize()
{
	bool Success =  Super::Initialize();
	if (!Success) return false;
	
	return true;
}
void UPingWidget::PreviewStart()
{
	//WidgetSwitcher->SetActiveWidgetIndex(0);
}

void UPingWidget::ServerPing()
{
	FVector2D Size = FVector2D(180.0f, 180.f);
	FSlateBrush Brush;
	switch (PingType)
	{
	case EPingTypes::Danger:
		Brush.SetResourceObject(DangerTexture);
		break;
	case EPingTypes::Help:
		Brush.SetResourceObject(HelpTexture);
		break;
	case EPingTypes::Missing:
		Brush.SetResourceObject(MissingTexture);
		break;
	case EPingTypes::Go:
		Brush.SetResourceObject(GoTexture);
		break;
	case EPingTypes::Basic:
		Brush.SetResourceObject(DefaultTexture);
		break;
	default:
		Brush.SetResourceObject(DefaultTexture);
		break;
	}
	Brush.ImageSize = Size; // 크기 조절필요
	FinalPing->SetBrush(Brush);
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void UPingWidget::GetDistaneOwnerToWidget(float _distance)
{
	//GEngine->AddOnScreenDebugMessage(6,1.0f,  FColor::Emerald, FString::Printf(TEXT("Distance : %f"), _distance));

	FVector2D Size = FVector2D(1, 1);
	FWidgetTransform widgetTransform;
	
	// 1000, 600, 400, 200
	if (_distance < 200)
	{
		Size = FVector2D(0.2,0.2);
	}
	else if (_distance < 400)
	{
		Size = FVector2D(0.4,0.4);
	
	}
	else if (_distance < 600)
	{
		Size = FVector2D(0.6,0.6);
	}
	else// 601 ~ 1000
	{
		Size = FVector2D(1,1);
		
	}
	widgetTransform.Scale = Size;
	WidgetSwitcher->SetRenderTransform(widgetTransform);
}

void UPingWidget::ImageInitailzie()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
	PingType = EPingTypes::Basic;
	BP_ImageInit();
}

void UPingWidget::SetPingType(EPingTypes _type)
{
	if (PingType != _type)
	{
		PingType = _type;
		switch (PingType)
		{
			case EPingTypes::Danger:
			BP_HighLight("Danger");
			break;
			case EPingTypes::Help:
		BP_HighLight("Help");
			break;
			case EPingTypes::Missing:
		BP_HighLight("Missing");
			break;
			case EPingTypes::Go:
		BP_HighLight("Go");
			break;
		case EPingTypes::Basic:
			break;
		}
	}
}

void UPingWidget::BroadCastPing_Implementation()
{
	ServerPing();
}


