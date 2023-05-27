// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModelViewer : ModuleRules
{
	public ModelViewer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "RawMesh","ImageWrapper","Slate",
			"MeshDescription", "StaticMeshDescription", "MeshConversion", "ProceduralMeshComponent" });
	}
}
