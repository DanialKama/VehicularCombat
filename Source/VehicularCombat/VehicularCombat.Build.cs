// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VehicularCombat : ModuleRules
{
	public VehicularCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ChaosVehicles", "HeadMountedDisplay", "PhysicsCore", "NavigationSystem" });

		PublicDefinitions.Add("HMD_MODULE_INCLUDED=1");
	}
}
