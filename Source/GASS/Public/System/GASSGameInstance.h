// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameInstance.h"

#include "GASSGameInstance.generated.h"

class AGASSPlayerController;
class UObject;

UCLASS(Config = Game)
class GASS_API UGASSGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	UGASSGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	AGASSPlayerController* GetPrimaryPlayerController() const;
	
	virtual bool CanJoinRequestedSession() const override;

	virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

protected:

	virtual void Init() override;
	virtual void Shutdown() override;

	void OnPreClientTravelToSession(FString& URL);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};
