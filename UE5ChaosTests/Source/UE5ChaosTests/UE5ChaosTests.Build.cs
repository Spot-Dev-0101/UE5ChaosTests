// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE5ChaosTests : ModuleRules
{
	public UE5ChaosTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { "GeometryCollectionEngine", "Chaos", "ChaosSolverEngine", "PhysicsCore" });
	}
}
