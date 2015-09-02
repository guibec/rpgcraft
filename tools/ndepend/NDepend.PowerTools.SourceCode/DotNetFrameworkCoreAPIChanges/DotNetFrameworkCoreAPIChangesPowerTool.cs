
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.DotNet;
using NDepend.Path;
using NDepend.PowerTools.APIChanges;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;

namespace NDepend.PowerTools.DotNetFrameworkCoreAPIChanges {
   class DotNetFrameworkCoreAPIChangesPowerTool : IPowerTool {

      public string Name { get { return ".NET Framework v3.5 and v4.X installed, Core Public API Changes"; } }

      public string[] Description {
         get { return new string[] {
            "Analyze .NET Framework v3.5 and v4.X installed, core assemblies",
            "and report API changes.",
            "v4.X can be v4.0 or v4.5 depending which version is installed.",
            "v4.5 doesn't live side-by-side with v4.0, it replaces it."
         }; }
      }



      public void Run() {
         var projectManager = new NDependServicesProvider().ProjectManager;
         var dotNetManager = new NDependServicesProvider().DotNetManager;

         // Check installation
         var versions = new[] { new Version(3,5), new Version(4,0) };
         foreach (var version in versions) {
            if (dotNetManager.IsInstalled(DotNetProfile.DotNetFramework, version)) { continue; }
            Console.WriteLine(".NET fx v" + version.ToString() + " not installed!");
            return;
         }

         //
         // Gather core assemblies paths
         //
         var olderAssembliesPaths = GetCoreAssembliesPath(versions[0], dotNetManager);
         var newerAssembliesPaths = GetCoreAssembliesPath(versions[1], dotNetManager);

         //
         // Do analysis
         //
         var olderCodeBase = DoAnalysisGetCodeBase(versions[0], olderAssembliesPaths, TemporaryProjectMode.TemporaryOlder, projectManager);
         var newerCodeBase = DoAnalysisGetCodeBase(versions[1], newerAssembliesPaths, TemporaryProjectMode.TemporaryNewer, projectManager);

         //
         // Create compare context
         //
         var compareContext = newerCodeBase.CreateCompareContextWithOlder(olderCodeBase);

         ConsoleUtils.ShowNLinesOnConsole(3, ConsoleColor.Black);
         Console.WriteLine("2 temporary projects have been created.");
         Console.WriteLine("The .NET Fx older and newer analysis results can now be consummed live from the NDepend UI.");
         Console.WriteLine("Note that .NET Fx types API Breaking Changes represents types moved from System to mscorlib.");

         //
         // Show API Changes!
         //
         APIChangesDisplayer.Go(compareContext);
      }

      private static ICollection<IAbsoluteFilePath> GetCoreAssembliesPath(Version version, IDotNetManager dotNetManager) {
         Debug.Assert(version != null);
         Debug.Assert(dotNetManager != null);
         var list = new List<IAbsoluteFilePath>();
         var dirs = dotNetManager.GetDotNetProfileDirectories(DotNetProfile.DotNetFramework, version);
         list.AddRange(
            dirs.SelectMany(dir => new[] { "mscorlib.dll", "System.dll", "System.Core.dll", "System.Xml.dll" },
                            (dir, assemblyFileName) => dir.GetChildFileWithName(assemblyFileName))
           .Where(assemblyPath => assemblyPath.Exists));
         Debug.Assert(list.Count == 4);
         return list;
      }

      private static ICodeBase DoAnalysisGetCodeBase(
            Version version, 
            ICollection<IAbsoluteFilePath> assembliesPaths, 
            TemporaryProjectMode temporaryProjectMode, 
            IProjectManager projectManager) {
         Debug.Assert(version != null);
         Debug.Assert(assembliesPaths != null);
         Debug.Assert(projectManager != null);
         Console.WriteLine("Analyze .NET fx v" + version.ToString() + " core assemblies");
         var project = projectManager.CreateTemporaryProject(assembliesPaths, temporaryProjectMode);
         var analysisResult = ProjectAnalysisUtils.RunAnalysisShowProgressOnConsole(project);
         return analysisResult.CodeBase;
      }



   }
}
