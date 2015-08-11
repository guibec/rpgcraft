#if Unity_4_0

using UnityEditor;
using UnityEngine;
using System.Collections;

public class PackageIcons
{
    // This code is only used to package icons into an asset to be included in the distribution
    // It only runs under unity 4.X,  Tha AssetBundle code has changed under unity 5.X
    // We build p4connect in Unity 4.X so it is portable to either 4.X or 5.X

	[MenuItem("Assets/PackageIcons")]
	static void DoPackageIcons()
	{
		// Build the resource file from the active selection.
		string [] assetNames = new string[Selection.objects.Length];

		for (int i = 0; i < Selection.objects.Length; ++i)
		{
			assetNames[i] = System.IO.Path.GetFileName(Selection.objects[i].name);
		}
		BuildPipeline.BuildAssetBundleExplicitAssetNames(Selection.objects, assetNames, "Assets/P4Connect/Editor/Icons.pack", BuildAssetBundleOptions.UncompressedAssetBundle);
	}
}
#endif
