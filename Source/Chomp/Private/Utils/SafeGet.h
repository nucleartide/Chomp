#pragma once

class FSafeGet
{
public:
	template <typename T>
	static T* Pawn(const AController* Controller);

	static UWorld* World(const AActor* Actor);

	static APlayerController* PlayerController(const AActor* Actor, int PlayerIndex);

	template <typename T>
	static T* GameState(const AActor* Actor);
};
