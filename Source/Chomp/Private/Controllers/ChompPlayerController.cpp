#include "Controllers/ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"

AChompPlayerController::AChompPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompPlayerController::OnMoveVertical(float Input)
{
    VerticalAxisInput = Input;
}

void AChompPlayerController::OnMoveHorizontal(float Input)
{
    HorizontalAxisInput = Input;
}

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    auto ChompPawn = GetPawn<AChompPawn>();
    if (!ChompPawn)
        return;

    auto IsGamePlaying = GetWorld()->GetGameState<AChompGameState>()->GetEnum() == EChompGameState::Playing;
    if (!IsGamePlaying)
        return;

    FGridLocation IntendedDirection{
        FGenericPlatformMath::RoundToInt(VerticalAxisInput),
        FGenericPlatformMath::RoundToInt(HorizontalAxisInput)};

    if (!IsTargetTileSet)
    {
        auto LevelInstance = ULevelLoader::GetInstance(Level);
        auto ActorLocation = ChompPawn->GetActorLocation();
        auto ExcludedEntities = ChompPawn->GetExcludedEntities();
        auto IsPassable = LevelInstance->ComputeTargetTile(ActorLocation, IntendedDirection, ExcludedEntities, TargetTile);
        if (IsPassable)
            IsTargetTileSet = true;
    }

    if (IsTargetTileSet)
    {
        if (ChompPawn->MoveTowardsPoint(TargetTile, DeltaTime))
        {
            IsTargetTileSet = false;
        }
    }
}

void AChompPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Bind input axes.
    InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
    InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);

    // Disable this. We want to manage the active camera ourselves.
    bAutoManageActiveCameraTarget = false;
}
