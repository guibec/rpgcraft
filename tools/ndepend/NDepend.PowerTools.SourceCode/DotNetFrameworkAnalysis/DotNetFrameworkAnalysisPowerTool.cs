using System;
using System.Linq;
using NDepend.DotNet;
using NDepend.PowerTools.Base;
using NDepend.Project;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.AnalyzeDotNetFramework {
   class DotNetFrameworkAnalysisPowerTool : IPowerTool {
      public string Name { get { return "Analyze all .NET Framework Assemblies"; } }

      public string[] Description {
         get { return new string[] {
            "Gather all assemblies of the most recent .NET Fx installation on the disk...",
            "...and analyze them!",
         }; }
      }

      public void Run() {
         var projectManager = new NDependServicesProvider().ProjectManager;
         var dotNetManager = new NDependServicesProvider().DotNetManager;

         // Gather most recent .NET Fx + directories containing assemblies
         var mostRecentVersion = dotNetManager.GetDotNetProfileVersionsInstalled(DotNetProfile.DotNetFramework).First();
         var dirs = dotNetManager.GetDotNetProfileDirectories(DotNetProfile.DotNetFramework, mostRecentVersion);
         Console.WriteLine(".NET Fx v" + mostRecentVersion + " directories that contains assemblies to analyze:");
         foreach (var dir in dirs) {
            Console.WriteLine("  -> " + dir.ToString());
         }

         // Gather all .NET Fx assembly
         var dotNetFxAssemblies = (from dir in dirs
                                   from file in dir.ChildrenFilesPath
                                   where dotNetManager.IsAssembly(file)
                                   select file).ToList();

         Console.WriteLine(dotNetFxAssemblies.Count + " .NET Fx assemblies found");

         // Create temporary NDepend project and analyze it
         var project = projectManager.CreateTemporaryProject(dotNetFxAssemblies, TemporaryProjectMode.Temporary);
         var analysisResult = ProjectAnalysisUtils.RunAnalysisShowProgressOnConsole(project);

         // Show results
         ProjectAnalysisUtils.ShowConsoleAnalysisResult(analysisResult.CodeBase);

         Console.WriteLine("The .NET Fx analysis result produced can now be consummed live from the NDepend project named \"" + project.Properties.Name + "\"");
      }



   }
}
