
using System;
using System.Collections.Generic;
using NDepend.Helpers;
using NDepend.Path;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;
using NDepend.DotNet.VisualStudio;

namespace NDepend.PowerTools.AnalyzeCodeOnDisk {
   class AnalyzeCodeOnDiskPowerTool : IPowerTool {
      public string Name { get { return "Analyze Code on my Disk"; } }

      public string[] Description {
         get {
            return new [] {
            "Gather Visual Studio solutions and projects from Visual Studio",
            "Most Recently Used (MRU) list, for each Visual Studio installed version.",
            "Gather all Debug assemblies from these solutions and analyze them.",
         };
         }
      }

      public void Run() {
         var projectManager = new NDependServicesProvider().ProjectManager;
         var visualStudioManager = new NDependServicesProvider().VisualStudioManager;

         //
         // Gather all Most Recent Used VS solutions and projects file path referenced in registry 
         //
         var vsFilesPath = new List<IAbsoluteFilePath>();
         foreach (var vsVersion in new[] { VisualStudioVersion.V7_2003, VisualStudioVersion.V8_2005, VisualStudioVersion.V9_2008, VisualStudioVersion.V10_2010 }) {
            if (!visualStudioManager.IsVisualStudioVersionIntalled(vsVersion)) { continue; }
            vsFilesPath.AddRange(visualStudioManager.GetMostRecentlyUsedVisualStudioSolutionOrProject(vsVersion));
         }
         Console.WriteLine(vsFilesPath.Count + " VS sln or proj files found");

         //
         // Gather all Debug assemblies built from these VS solutions and projects
         //
         var assembliesPath = new List<IAbsoluteFilePath>();
         foreach (var vsFilePath in vsFilesPath) {
            assembliesPath.AddRange(visualStudioManager.GetAssembliesFromVisualStudioSolutionOrProject(vsFilePath));
         }

         //
         // Create temporary project
         //
         var project = projectManager.CreateTemporaryProject(assembliesPath, TemporaryProjectMode.Temporary);

         //
         // Inform about potential assembly resolving error
         //
         IReadOnlyList<IAbsoluteFilePath> applicationAssemblies, thirdPartyAssemblies;
         IReadOnlyList<IAssemblyResolvingError> applicationAssembliesError, thirdPartyAssembliesError;
         project.CodeToAnalyze.Resolve(out applicationAssemblies, out thirdPartyAssemblies, out applicationAssembliesError, out thirdPartyAssembliesError);
         if (applicationAssembliesError.Count > 0) {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine(applicationAssembliesError.Count + " assembly resolving error, assemblies with following names won't be analyzed:");
            foreach (var assemblyResolvingError in applicationAssembliesError) {
               // You can use assemblyResolvingError.ErrorDescription to show error in plain english 
               // most of the time it'll be several different assemblies with the same name found.
               Console.Write(assemblyResolvingError.AssemblyName + " ; ");
            }
            Console.ForegroundColor = ConsoleColor.White;
         }

         //
         // Run analysis
         //
         var analysisResult = ProjectAnalysisUtils.RunAnalysisShowProgressOnConsole(project);

         //
         // Show results
         //
         ProjectAnalysisUtils.ShowConsoleAnalysisResult(analysisResult.CodeBase);


         Console.WriteLine("The analysis result produced can now be consummed from the NDepend project \"" + project.Properties.Name + "\"");
      }
   }
}
