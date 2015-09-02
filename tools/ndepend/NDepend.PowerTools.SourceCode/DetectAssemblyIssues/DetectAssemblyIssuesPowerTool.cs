using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.DotNet;
using NDepend.Helpers;
using NDepend.Path;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;

namespace NDepend.PowerTools.DetectAssemblyIssues {
   class DetectAssemblyIssuesPowerTool : IPowerTool {
      public string Name { get { return "Detect Assemblies Versionning and Deployment Issues"; } }

      public string[] Description {
         get { 
            return new [] {
               "Check several potentials deployment issues,", 
               "including making sure that all assemblies and their references share the same version number."
            }; 
         }
      }



      public void Run() {

         IProject project;
         if (!ProjectAnalysisUtils.TryChooseProject(out project)) { return; }
         Debug.Assert(project != null);

         IReadOnlyList<IAbsoluteFilePath> applicationAssemblies, thirdPartyAssemblies;
         IReadOnlyList<IAssemblyResolvingError> applicationAssembliesError, thirdPartyAssembliesError;
         project.CodeToAnalyze.Resolve(out applicationAssemblies, out thirdPartyAssemblies, out applicationAssembliesError, out thirdPartyAssembliesError);


         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(ExtensionMethods.ToEnumerable("Show eventual assembly resolving errors"));
         ShowEventualAssemblyResolvingErrors(applicationAssemblies.Count, applicationAssembliesError);



         //--------------------------------------------------------------------------------------------
         // Make sure there are enought application assemblies resolved
         if(applicationAssemblies.Count < 3) {
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("Only " + applicationAssemblies.Count + " application assembly are resolved, this is not enought to try detect versionning issue.");
            return;
         }
         var dotNetManager = new NDependServicesProvider().DotNetManager;
         var applicationAssembliesInfos = applicationAssemblies.Select(dotNetManager.GetAssemblyInfo);
         var thirdPartyAssembliesInfos = thirdPartyAssemblies.Select(dotNetManager.GetAssemblyInfo);



         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(new[] {"Try get the application version", "and check assemblies without the application version" });
         CoherencyChecker.Go(applicationAssembliesInfos,
                             assemblyInfo => assemblyInfo.Version,
                             "application version");



         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(ExtensionMethods.ToEnumerable("Check that assemblies references have the same version than assemblies resolved"));
         AssembliesReferencesVersionningChecker.Go(applicationAssembliesInfos, thirdPartyAssembliesInfos);


         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(new[] {"Try get the common target runtime version", "and check assemblies with a different target runtime version" });
         CoherencyChecker.Go(applicationAssembliesInfos,
                             assemblyInfo => assemblyInfo.TargetRuntime,
                             "target runtime version");



         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(ExtensionMethods.ToEnumerable("Check that all application assemblies are strong named or not"));
         CoherencyChecker.Go(applicationAssembliesInfos,
                             assemblyInfo => assemblyInfo.IsStrongNamed,
                             "is strong named");


         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(new[] {"Check that all application assemblies share ", "a common target platform target (AnyCPU, x86, x64, Itanium)"});
         CoherencyChecker.Go(applicationAssembliesInfos,
                             assemblyInfo => assemblyInfo.PlatformTarget,
                             "platform target");


         //--------------------------------------------------------------------------------------------
         ShowConsoleNewCheck(new[] {"Make sure all application assemblies", "PDB file available or not available"});
         CoherencyChecker.Go(applicationAssembliesInfos,
                             assemblyInfo => assemblyInfo.PDBAvailable,
                             "PDB file available");
      }


      private static void ShowConsoleNewCheck(IEnumerable<string> checkDescLines) {
         Debug.Assert(checkDescLines != null);
         Console.ForegroundColor = ConsoleColor.Cyan;
         ConsoleUtils.ShowNLinesOnConsole(2, ConsoleColor.Black);
         Console.WriteLine("//" + new string('-', Console.WindowWidth-3));
         Console.WriteLine("//");
         foreach (var line in checkDescLines) {
            Console.WriteLine("// " + line);   
         }
         Console.WriteLine("//");
      }




      //
      // Show eventual assembly resolving errors
      //
      private static void ShowEventualAssemblyResolvingErrors(int applicationAssembliesResolvedCount, IReadOnlyList<IAssemblyResolvingError> applicationAssembliesError) {
         Debug.Assert(applicationAssembliesError != null);
         if (applicationAssembliesError.Count == 0) {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("All " + applicationAssembliesResolvedCount + " application assemblies resolved!");
         } else {
            if (applicationAssembliesResolvedCount > 0) {
               Console.ForegroundColor = ConsoleColor.Green;
               Console.WriteLine(applicationAssembliesResolvedCount + " application assemblies resolved!");
            }
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine(applicationAssembliesError.Count + " application assembly resolving error.");
            foreach (var assemblyResolvingError in applicationAssembliesError) {
               Console.WriteLine(" -> " + assemblyResolvingError.AssemblyName + " : " + assemblyResolvingError.ErrorDescription);
            }
         }
         Console.ForegroundColor = ConsoleColor.White;
         Console.WriteLine();
      }

   }
}
