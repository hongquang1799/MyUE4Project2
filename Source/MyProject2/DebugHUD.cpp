// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugHUD.h"
#include "Engine/Public/CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "ConstructorHelpers.h"

ADebugHUD::ADebugHUD()
{
}

void ADebugHUD::DrawHUD()
{
	Super::DrawHUD();
	float ScaleUI = Canvas->ClipY / 1080.0f;
	float TextScale = 1.f;
	ShowInfoItems(0.f, ScaleUI, TextScale);
}

void ADebugHUD::BeginPlay()
{
	Super::BeginPlay();

	if (WidgetClass != NULL)
	{
		auto Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		Widget->AddToViewport();
	}
}

void ADebugHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADebugHUD::AddText(const FString & Text, const FColor & Color, float Duration)
{
	FInfoItem InfoItem;
	InfoItem.ExpireTime = GetWorld()->RealTimeSeconds + Duration;
	TSharedPtr<FCanvasTextItem> TextItem(new FCanvasTextItem(FVector2D(0, 0), FText::FromString(Text), GEngine->GetSmallFont(), Color));
	TextItem->EnableShadow(FLinearColor::Black);
	TextItem->BlendMode = SE_BLEND_Translucent;
	FFontRenderInfo fri = FFontRenderInfo();
	fri.bClipText = true;
	TextItem->FontRenderInfo = fri;
	InfoItem.TextItem = TextItem;
	InfoItems.Add(InfoItem);
}

float ADebugHUD::ShowInfoItems(float YOffset, float ScaleUI, float TextScale)
{
	float Y = YOffset;
	float CanvasCentre = Canvas->ClipX / 2.0f; //Centered on the screen, set to zero or some other value if you it left-aligned

	bool ContainsExpiredMessage = false;
	for (int32 iItem = 0; iItem < InfoItems.Num(); ++iItem)
	{
		float X = 0.0f;
		float SizeX, SizeY;
		FInfoItem& InfoItem = InfoItems[iItem];
		if (InfoItem.ExpireTime < GetWorld()->RealTimeSeconds)
		{
			ContainsExpiredMessage = true;
			InfoItem.Expired = true;
		}
		else
		{
			if (InfoItem.TextItem.IsValid())
			{
				Canvas->StrLen(InfoItem.TextItem->Font, InfoItem.TextItem->Text.ToString(), SizeX, SizeY);
				//Center
				//X = CanvasCentre - (SizeX * InfoItem.TextItem->Scale.X) / 2.0f;
				//Right align
				X = Canvas->ClipX - (SizeX * InfoItem.TextItem->Scale.X);
				Canvas->DrawItem((*(InfoItem.TextItem.Get())), X, Y);
				Y += SizeY * InfoItem.TextItem->Scale.Y;
			}
		}
	}

	//If there were any expired messages then we need to safely remove them
	if (ContainsExpiredMessage)
	{
		float RealTimeSeconds = GetWorld()->RealTimeSeconds;
		InfoItems.RemoveAll([&](FInfoItem InfoItem) {
			return InfoItem.ExpireTime < RealTimeSeconds;
		});
	}

	return Y;
}
