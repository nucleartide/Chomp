#include "Controllers/ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"

AChompPlayerController::AChompPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompPlayerController::OnMoveVertical(float Delta)
{
    HorizontalAxisDelta = 0;
    VerticalAxisDelta = Delta;
}

void AChompPlayerController::OnMoveHorizontal(float Delta)
{
    HorizontalAxisDelta = Delta;
    VerticalAxisDelta = 0;
}

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if pawn is alive.
    auto Pawn = GetPawn<AChompPawn>();
    if (!Pawn)
    {
        // Then pawn is most likely dead. Early return.
        return;
    }

    // Move the pawn using our MovementDirection.
    FGridLocation MovementDir{FGenericPlatformMath::RoundToInt(VerticalAxisDelta), (int)FGenericPlatformMath::RoundToInt(HorizontalAxisDelta)}; // Note that HorizontalAxisDelta / VerticalAxisDelta are either 0 or 1.
    Pawn->MoveTowards(MovementDir, DeltaTime);
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
