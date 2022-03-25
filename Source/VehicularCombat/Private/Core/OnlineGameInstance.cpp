// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Core/OnlineGameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"

UOnlineGameInstance::UOnlineGameInstance()
{
	// CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleted));
	// UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnUpdateSessionCompleted));
	// UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnUpdateSessionCompleted));
	// CreateSessionCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnCreateSessionCompleted);
	// UpdateSessionCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnUpdateSessionCompleted);
	// StartSessionCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnStartSessionCompleted);
	// EndSessionCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnEndSessionCompleted);
	// DestroySessionCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnDestroySessionCompleted);
	// FindSessionsCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnFindSessionsCompleted);
	// JoinSessionCompleteDelegate.BindUObject(this, &UOnlineGameInstance::OnJoinSessionCompleted);

	// Initialize variables
	LevelName = FString("/Game/Maps/TestMaps/Test?listen");
}

void UOnlineGameInstance::Init()
{
	IOnlineSubsystem * Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface)
		{
			// Bind delegates
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstance::OnCreateSessionCompleted);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UOnlineGameInstance::OnFindSessionsCompleted);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstance::OnJoinSessionCompleted);
		}
	}
}

void UOnlineGameInstance::CreateSession(FCreateServerInfo InCreateServerInfo)
{
	if (SessionInterface.IsValid() == false)
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (InCreateServerInfo.ServerName != "")
	{
		SessionSettings = MakeShareable(new FOnlineSessionSettings);
		SessionSettings->NumPrivateConnections = 0;
		SessionSettings->NumPublicConnections = InCreateServerInfo.MaxPlayers;
		SessionSettings->bAllowInvites = true;
		SessionSettings->bAllowJoinInProgress = true;
		SessionSettings->bAllowJoinViaPresence = true;
		SessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
		SessionSettings->bIsDedicated = false;
		SessionSettings->bUsesPresence = true;
		SessionSettings->bShouldAdvertise = true;

		// CreateServerInfo = InCreateServerInfo;
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSettings->bIsLANMatch = true;
		}
		else
		{
			SessionSettings->bIsLANMatch = InCreateServerInfo.bIsLAN;
		}
	
		SessionSettings->Set(FName("SERVER_NAME_KEY"), InCreateServerInfo.ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
		SessionInterface->CreateSession(0, FName("My Session"), *SessionSettings);
	}
	else
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
	}
}

void UOnlineGameInstance::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	// if (SessionInterface)
	// {
	// 	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	// }

	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(LevelName);
	}
}

void UOnlineGameInstance::FindSessions()
{
	ServerSearchState.Broadcast(true);
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		SessionSearch->bIsLanQuery = true;
	}
	else
	{
		SessionSearch->bIsLanQuery = false;
	}
	
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->PingBucketSize = 50;
	
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UOnlineGameInstance::OnFindSessionsCompleted(bool Successful)
{
	// if (SessionInterface)
	// {
	// 	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	// }
	
	// if (LastSessionSearch->SearchResults.Num() <= 0)
	// {
	// 	OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), Successful);
	// 	return;
	// }
	//
	// OnFindSessionsCompleteEvent.Broadcast(LastSessionSearch->SearchResults, Successful);
	
	if (Successful)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		if (SearchResults.Num() > 0)
		{
			for (int32 i  = 0; i < SearchResults.Num(); ++i)
			{
				if (SearchResults[i].IsValid() == false)
				{
					continue;
				}
				
				FServerInfo ServerInfo;
				SearchResults[i].Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerInfo.ServerName);
				ServerInfo.MaxPlayers = SearchResults[i].Session.NumOpenPublicConnections;
				ServerInfo.CurrentPlayers = ServerInfo.MaxPlayers - SearchResults[i].Session.NumOpenPublicConnections;
				ServerInfo.bIsLAN = SearchResults[i].Session.SessionSettings.bIsLANMatch;
				ServerInfo.Ping = SearchResults[i].PingInMs;
				ServerInfo.ServerIndex = i;
				ServerInfoDelegate.Broadcast(ServerInfo);
			}
		}
	}
	
	ServerSearchState.Broadcast(false);
}

void UOnlineGameInstance::TryJoinSession(FName SessionName, int32 ServerIndex)
{
	const FOnlineSessionSearchResult Result = SessionSearch->SearchResults[ServerIndex];
	if (Result.IsValid())
	{
		// const ULocalPlayer* const Player = GetFirstGamePlayer();
		SessionInterface->JoinSession(0, FName("My Session"), Result);
	}
}

void UOnlineGameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// if (SessionInterface)
	// {
	// 	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	// }
	//
	// OnJoinGameSessionCompleteEvent.Broadcast(Result);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
		if (JoinAddress != "")
		{
			PlayerController->ClientTravel(JoinAddress, TRAVEL_Absolute);
		}
	}
}

// void UOnlineGameInstance::CreateSession(int32 NumPublicConnections, bool IsLANMatch)
// {
// 	if (SessionInterface.IsValid() == false)
// 	{
// 		OnCreateSessionCompleteEvent.Broadcast(false);
// 		return;
// 	}
	
	// LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	// LastSessionSettings->NumPrivateConnections = 0;
	// LastSessionSettings->NumPublicConnections = NumPublicConnections;
	// LastSessionSettings->bAllowInvites = true;
	// LastSessionSettings->bAllowJoinInProgress = true;
	// LastSessionSettings->bAllowJoinViaPresence = true;
	// LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	// LastSessionSettings->bIsDedicated = false;
	// LastSessionSettings->bUsesPresence = true;
	// LastSessionSettings->bIsLANMatch = IsLANMatch;
	// LastSessionSettings->bShouldAdvertise = true;

	// SessionInterface->CreateSession(0, FName("My Session"), LastSessionSettings);
	
	// LastSessionSettings->Set(SETTING_MAPNAME, LevelName, EOnlineDataAdvertisementType::ViaOnlineService);
	//
	// CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	//
	// const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// if (SessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings) == false)
	// {
	// 	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	// 	
	// 	OnCreateSessionCompleteEvent.Broadcast(false);
	// }
// }

void UOnlineGameInstance::UpdateSession()
{
	if (SessionInterface.IsValid() == false)
	{
		OnUpdateSessionCompleteEvent.Broadcast(false);
		return;
	}
	
	TSharedPtr<FOnlineSessionSettings> updatedSessionSettings = MakeShareable(new FOnlineSessionSettings(*LastSessionSettings));
	updatedSessionSettings->Set(SETTING_MAPNAME, LevelName, EOnlineDataAdvertisementType::ViaOnlineService);
	
	UpdateSessionCompleteDelegateHandle = SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegate);
	
	if (SessionInterface->UpdateSession(NAME_GameSession, *updatedSessionSettings) == false)
	{
		SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);

		OnUpdateSessionCompleteEvent.Broadcast(false);
	}
	else
	{
		LastSessionSettings = updatedSessionSettings;
	}
}

void UOnlineGameInstance::OnUpdateSessionCompleted(FName SessionName, bool Successful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	}
	
	OnUpdateSessionCompleteEvent.Broadcast(Successful);
}

void UOnlineGameInstance::StartSession()
{
	if (SessionInterface.IsValid() == false)
	{
		OnStartSessionCompleteEvent.Broadcast(false);
		return;
	}
	
	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
	
	if (SessionInterface->StartSession(NAME_GameSession) == false)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		
		OnStartSessionCompleteEvent.Broadcast(false);
	}
}

void UOnlineGameInstance::OnStartSessionCompleted(FName SessionName, bool Successful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}
	
	OnStartSessionCompleteEvent.Broadcast(Successful);
}

void UOnlineGameInstance::EndSession()
{
	if (SessionInterface.IsValid() == false)
	{
		OnEndSessionCompleteEvent.Broadcast(false);
		return;
	}
	
	EndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegate);
	
	if (SessionInterface->EndSession(NAME_GameSession) == false)
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		
		OnEndSessionCompleteEvent.Broadcast(false);
	}
}

void UOnlineGameInstance::OnEndSessionCompleted(FName SessionName, bool Successful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
	}
	
	OnEndSessionCompleteEvent.Broadcast(Successful);
}

void UOnlineGameInstance::DestroySession()
{
	if (SessionInterface.IsValid() == false)
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}
	
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (SessionInterface->DestroySession(NAME_GameSession) == false)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		
		OnDestroySessionCompleteEvent.Broadcast(false);
	}
}

void UOnlineGameInstance::OnDestroySessionCompleted(FName SessionName, bool Successful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	
	OnDestroySessionCompleteEvent.Broadcast(Successful);
}

void UOnlineGameInstance::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	if (SessionInterface.IsValid() == false)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IsLANQuery;
	
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (SessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()) == false)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		
		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

// void UOnlineGameInstance::OnFindSessionsCompleted(bool Successful)
// {
// 	if (SessionInterface)
// 	{
// 		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
// 	}
// 	
// 	if (LastSessionSearch->SearchResults.Num() <= 0)
// 	{
// 		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), Successful);
// 		return;
// 	}
// 	
// 	OnFindSessionsCompleteEvent.Broadcast(LastSessionSearch->SearchResults, Successful);
// }

// void UOnlineGameInstance::JoinGameSession(const FOnlineSessionSearchResult& SessionResult)
// {
// 	if (SessionInterface.IsValid() == false)
// 	{
// 		OnJoinGameSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
// 		return;
// 	}
// 	
// 	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
// 	
// 	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
// 	if (SessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult) == false)
// 	{
// 		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
// 		
// 		OnJoinGameSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
// 	}
// }

// void UOnlineGameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
// {
// 	if (SessionInterface)
// 	{
// 		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
// 	}
// 	
// 	OnJoinGameSessionCompleteEvent.Broadcast(Result);
// }

bool UOnlineGameInstance::TryTravelToCurrentSession()
{
	if (SessionInterface.IsValid() == false)
	{
		return false;
	}
	
	FString connectString;
	if (SessionInterface->GetResolvedConnectString(NAME_GameSession, connectString) == false)
	{
		return false;
	}
	
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	playerController->ClientTravel(connectString, TRAVEL_Absolute);
	return true;
}

//
// UOnlineGameInstance::UOnlineGameInstance(const FObjectInitializer& ObjectInitializer)
// 	: Super(ObjectInitializer)
// {
// 	/** Bind function for CREATING a Session */
// 	// OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnCreateSessionComplete);
// 	// OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnStartOnlineGameComplete);
// 	//
// 	// /** Bind function for FINDING a Session */
// 	// OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnFindSessionsComplete);
// 	//
// 	// /** Bind function for JOINING a Session */
// 	// OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnJoinSessionComplete);
// 	//
// 	// /** Bind function for DESTROYING a Session */
// 	// OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnDestroySessionComplete);
//
// 	// Initialize variables
// 	LevelName = FName("NewMap");
// }
//
// void UOnlineGameInstance::StartOnlineGame()
// {
// 	// Creating a local player where we can get the UserID from
// 	const ULocalPlayer* const Player = GetFirstGamePlayer();
// 	
// 	// Call our custom HostSession function. GameSessionName is a GameInstance variable
// 	HostSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, true, true, 4);
// }
//
// void UOnlineGameInstance::FindOnlineGames()
// {
// 	const ULocalPlayer* const Player = GetFirstGamePlayer();
//
// 	FindSessions(Player->GetPreferredUniqueNetId().GetUniqueNetId(), true, true);
// }
//
// void UOnlineGameInstance::JoinOnlineGame()
// {
// 	const ULocalPlayer* const Player = GetFirstGamePlayer();
//
// 	// If the Array is not empty, we can go through it
// 	if (SessionSearch->SearchResults.Num() > 0)
// 	{
// 		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
// 		{
// 			// To avoid something crazy, we filter sessions from ourself
// 			if (SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId())
// 			{
// 				// Just a SearchResult where we can save the one we want to use, for the case we find more than one!
// 				const FOnlineSessionSearchResult SearchResult = SessionSearch->SearchResults[i];
//
// 				// Once we found source a Session that is not ours, just join it. Instead of using a for loop, you could
// 				// use a widget where you click on and have a reference for the GameSession it represents which you can use here
// 				JoinSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SearchResult);
// 				break;
// 			}
// 		}
// 	}	
// }
//
// void UOnlineGameInstance::DestroySessionAndLeaveGame()
// {
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
// 	if (OnlineSub)
// 	{
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid())
// 		{
// 			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
//
// 			Sessions->DestroySession(GameSessionName);
// 		}
// 	}
// }
//
// bool UOnlineGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
// {
// 	// Get the Online Subsystem to work with
// 	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
//
// 	if (OnlineSub)
// 	{
// 		// Get the Session Interface, so we can call the "CreateSession" function on it
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid() && UserId.IsValid())
// 		{
// 			/* 
// 				Fill in all the Session Settings that we want to use.
// 				
// 				There are more with SessionSettings.Set(...);
// 				For example the Map or the GameMode/Type.
// 			*/
// 			SessionSettings = MakeShareable(new FOnlineSessionSettings());
//
// 			SessionSettings->bIsLANMatch = bIsLAN;
// 			SessionSettings->bUsesPresence = bIsPresence;
// 			SessionSettings->NumPublicConnections = MaxNumPlayers;
// 			SessionSettings->NumPrivateConnections = 0;
// 			SessionSettings->bAllowInvites = true;
// 			SessionSettings->bAllowJoinInProgress = true;
// 			SessionSettings->bShouldAdvertise = true;
// 			SessionSettings->bAllowJoinViaPresence = true;
// 			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
//
// 			SessionSettings->Set(SETTING_MAPNAME, FString("NewMap"), EOnlineDataAdvertisementType::ViaOnlineService);
//
// 			// Set the delegate to the Handle of the SessionInterface
// 			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
//
// 			// Our delegate should get called when this is complete (doesn't need to be successful!)
// 			return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
// 		}
// 	}
// 	else
// 	{
// 		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
// 	}
//
// 	return false;
// }
//
// void UOnlineGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
// {
// 	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
//
// 	// Get the OnlineSubsystem so we can get the Session Interface
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
// 	if (OnlineSub)
// 	{
// 		// Get the Session Interface to call the StartSession function
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid())
// 		{
// 			// Clear the SessionComplete delegate handle, since we finished this call
// 			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
// 			if (bWasSuccessful)
// 			{
// 				// Set the StartSession delegate handle
// 				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
//
// 				// Our StartSessionComplete delegate should get called after this
// 				Sessions->StartSession(SessionName);
// 			}
// 		}
// 		
// 	}
// }
//
// void UOnlineGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
// {
// 	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
//
// 	// Get the Online Subsystem so we can get the Session Interface
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
// 	if (OnlineSub)
// 	{
// 		// Get the Session Interface to clear the Delegate
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
// 		if (Sessions.IsValid())
// 		{
// 			// Clear the delegate, since we are done with this call
// 			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
// 		}
// 	}
//
// 	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
// 	if (bWasSuccessful)
// 	{
// 		UGameplayStatics::OpenLevel(GetWorld(), "NewMap", true, "listen");
// 	}
// }
//
// void UOnlineGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
// {
// 	// Get the OnlineSubsystem we want to work with
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
//
// 	if (OnlineSub)
// 	{
// 		// Get the SessionInterface from our OnlineSubsystem
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid() && UserId.IsValid())
// 		{
// 			/*
// 				Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
// 			*/
// 			SessionSearch = MakeShareable(new FOnlineSessionSearch());
//
// 			SessionSearch->bIsLanQuery = bIsLAN;
// 			SessionSearch->MaxSearchResults = 20;
// 			SessionSearch->PingBucketSize = 50;
// 			
// 			// We only want to set this Query Setting if "bIsPresence" is true
// 			if (bIsPresence)
// 			{
// 				SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
// 			}
//
// 			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();
//
// 			// Set the Delegate to the Delegate Handle of the FindSession function
// 			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
// 			
// 			// Finally call the SessionInterface function. The Delegate gets called once this is finished
// 			Sessions->FindSessions(*UserId, SearchSettingsRef);
// 		}
// 	}
// 	else
// 	{
// 		// If something goes wrong, just call the Delegate Function directly with "false".
// 		OnFindSessionsComplete(false);
// 	}
// }
//
// void UOnlineGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
// {
// 	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));
//
// 	// Get OnlineSubsystem we want to work with
// 	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
// 	if (OnlineSub)
// 	{
// 		// Get SessionInterface of the OnlineSubsystem
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
// 		if (Sessions.IsValid())
// 		{
// 			// Clear the Delegate handle, since we finished this call
// 			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
//
// 			// Just debugging the Number of Search results. Can be displayed in UMG or something later on
// 			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));
// 		
// 			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
// 			if (SessionSearch->SearchResults.Num() > 0)
// 			{
// 				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
// 				// This can be customized later on with your own classes to add more information that can be set and displayed
// 				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
// 				{
// 					// OwningUserName is just the SessionName for now. I guess you can create your own Host Settings class and GameSession Class and add a proper GameServer Name here.
// 					// This is something you can't do in Blueprint for example!
// 					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx+1, *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)));
// 				}
// 			}
// 		}
// 	}
// }
//
// bool UOnlineGameInstance::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
// {
// 	// Return bool
// 	bool bSuccessful = false;
//
// 	// Get OnlineSubsystem we want to work with
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
//
// 	if (OnlineSub)
// 	{
// 		// Get SessionInterface from the OnlineSubsystem
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid() && UserId.IsValid())
// 		{
// 			// Set the Handle again
// 			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
// 			
// 			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
// 			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
// 			bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
// 		}
// 	}
// 		
// 	return bSuccessful;
// }
//
// void UOnlineGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
// {
// 	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));
//
// 	// Get the OnlineSubsystem we want to work with
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
// 	if (OnlineSub)
// 	{
// 		// Get SessionInterface from the OnlineSubsystem
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid())
// 		{
// 			// Clear the Delegate again
// 			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
//
// 			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
// 			// This is something the Blueprint Node "Join Session" does automatically!
// 			APlayerController * const PlayerController = GetFirstLocalPlayerController();
//
// 			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
// 			// String for us by giving him the SessionName and an empty String. We want to do this, because
// 			// Every OnlineSubsystem uses different TravelURLs
// 			FString TravelURL;
//
// 			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
// 			{
// 				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
// 				// how it really looks like
// 				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
// 			}
// 		}
// 	}
// }
//
// void UOnlineGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
// {
// 	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
//
// 	// Get the OnlineSubsystem we want to work with
// 	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
// 	if (OnlineSub)
// 	{
// 		// Get the SessionInterface from the OnlineSubsystem
// 		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
//
// 		if (Sessions.IsValid())
// 		{
// 			// Clear the Delegate
// 			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
//
// 			// If it was successful, we just load another level (could be a MainMenu!)
// 			if (bWasSuccessful)
// 			{
// 				UGameplayStatics::OpenLevel(GetWorld(), "ThirdPersonExampleMap", true);
// 			}
// 		}
// 	}
// }