// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DebugHUD.generated.h"

struct FInfoItem
{
	bool Expired;
	float ExpireTime;
	TSharedPtr<class FCanvasTextItem> TextItem;

	FInfoItem() : Expired(false), ExpireTime(0), TextItem(NULL) {}
};

/**
 *
 */
UCLASS()
class MYPROJECT2_API ADebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADebugHUD();

	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	/* Add text for printing debug / client only messages */
	void AddText(const FString& Text, const FColor& Color, float Duration);

	UPROPERTY(EditAnyWhere, Category = "Widget Class")
	TSubclassOf<class UUserWidget> WidgetClass;
protected:
	/** Array of information strings to render (Waiting to respawn etc) */
	TArray<FInfoItem> InfoItems;

	float ShowInfoItems(float YOffset, float ScaleUI, float TextScale);

};
