#pragma once

class FSafeGet
{
public:
	template <typename T>
	static T* Pawn(AController *Controller);

	static UWorld *World(AActor *Actor);
	
	static APlayerController *PlayerController(AActor *Actor, int PlayerIndex);
};
