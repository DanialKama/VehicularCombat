// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnCreateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnUpdateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnStartSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnEndSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnDestroySessionComplete, bool, Successful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCSOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FCSOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

// /** Delegate called when session created */
// FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
//
// /** Delegate called when session started */
// FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
//
// /** Handles to registered delegates for creating/starting a session */
// FDelegateHandle OnCreateSessionCompleteDelegateHandle;
// FDelegateHandle OnStartSessionCompleteDelegateHandle;
//
// /** Delegate for searching for sessions */
// FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
//
// /** Handle to registered delegate for searching a session */
// FDelegateHandle OnFindSessionsCompleteDelegateHandle;
//
// /** Delegate for joining a session */
// FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
//
// /** Handle to registered delegate for joining a session */
// FDelegateHandle OnJoinSessionCompleteDelegateHandle;
//
// /** Delegate for destroying a session */
// FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
//
// /** Handle to registered delegate for destroying a session */
// FDelegateHandle OnDestroySessionCompleteDelegateHandle;

UCLASS()
class VEHICULARCOMBAT_API UOnlineGameInstance : public UGameInstanceSubsystem
{
	GENERATED_BODY()

// Functions
public:
	UOnlineGameInstance();

	void CreateSession(int32 NumPublicConnections, bool IsLANMatch);
	
	FCSOnCreateSessionComplete OnCreateSessionCompleteEvent;

	void UpdateSession();
	
	FCSOnUpdateSessionComplete OnUpdateSessionCompleteEvent;

	void StartSession();
	
	FCSOnStartSessionComplete OnStartSessionCompleteEvent;

	void EndSession();
	
	FCSOnEndSessionComplete OnEndSessionCompleteEvent;

	void DestroySession();
	
	FCSOnDestroySessionComplete OnDestroySessionCompleteEvent;

	void FindSessions(int32 MaxSearchResults, bool IsLANQuery);
	
	FCSOnFindSessionsComplete OnFindSessionsCompleteEvent;

	void JoinGameSession(const FOnlineSessionSearchResult& SessionResult);
	
	FCSOnJoinSessionComplete OnJoinGameSessionCompleteEvent;

	// UFUNCTION(BlueprintCallable, Category = "Network")
	// void StartOnlineGame();
	//
	// UFUNCTION(BlueprintCallable, Category = "Network")
	// void FindOnlineGames();
	//
	// UFUNCTION(BlueprintCallable, Category = "Network")
	// void JoinOnlineGame();
	//
	// UFUNCTION(BlueprintCallable, Category = "Network")
	// void DestroySessionAndLeaveGame();

protected:
	void OnCreateSessionCompleted(FName SessionName, bool Successful);
	void OnUpdateSessionCompleted(FName SessionName, bool Successful);
	void OnStartSessionCompleted(FName SessionName, bool Successful);
	void OnEndSessionCompleted(FName SessionName, bool Successful);
	void OnDestroySessionCompleted(FName SessionName, bool Successful);
	void OnFindSessionsCompleted(bool Successful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	bool TryTravelToCurrentSession();

	// /**
	// *	Function to host a game!
	// *	@param	UserID			User that started the request
	// *	@param	SessionName		Name of the Session
	// *	@param	bIsLAN			Is this is LAN Game?
	// *	@param	bIsPresence		"Is the Session to create a presence Session"
	// *	@param	MaxNumPlayers	Number of Maximum allowed players on this "Session" (Server)
	// */
	// bool HostSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);
	//
	// /**
	// *	Find an online session
	// *	@param	UserId		user that initiated the request
	// *	@param	bIsLAN		are we searching LAN matches
	// *	@param	bIsPresence	are we searching presence sessions
	// */
	// void FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);
	//
	// /**
	// *	Joins a session via a search result
	// *	@param	SessionName		name of session
	// *	@param	SearchResult	Session to join
	// *	@return	bool true if successful, false otherwise
	// */
	// bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);
	//
	// /**
	// *	Delegate fired when a destroying an online session has completed
	// *	@param	SessionName		the name of the session this callback is for
	// *	@param	bWasSuccessful	true if the async action completed without error, false if there was an error
	// */
	// virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FDelegateHandle EndSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	
	// /**
	// *	Function fired when a session create request has completed
	// *	@param	SessionName		the name of the session this callback is for
	// *	@param	bWasSuccessful	true if the async action completed without error, false if there was an error
	// */
	// 	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	//
	// /**
	// *	Function fired when a session start request has completed
	// *	@param	SessionName		the name of the session this callback is for
	// *	@param	bWasSuccessful	true if the async action completed without error, false if there was an error
	// */
	// void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
	//
	// /**
	// *	Delegate fired when a session search query has completed
	// *	@param	bWasSuccessful	true if the async action completed without error, false if there was an error
	// */
	// void OnFindSessionsComplete(bool bWasSuccessful);
	//
	// /**
	// *	Delegate fired when a session join request has completed
	// *	@param	SessionName	the name of the session this callback is for
	// *	@param	Result		Result of joining the session
	// */
	// void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
// Variables
private:
	TSharedPtr<FOnlineSessionSettings> SessionSettings;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FName LevelName;
};