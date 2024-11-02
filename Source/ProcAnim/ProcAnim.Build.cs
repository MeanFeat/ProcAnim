// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProcAnim : ModuleRules
{
	public ProcAnim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"$(PluginDir)/Source/ProcAnim/Public/AnimAuthoring",
				"$(PluginDir)/Source/ProcAnim/Public/AnimCurve",
				"$(PluginDir)/Source/ProcAnim/Public/Common",
				"$(PluginDir)/Source/ProcAnim/Public/EditorExtensions",
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Sequencer", 
				"MovieSceneTools",
				"CurveEditor", 
				"Eigen",
				"MLES",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
