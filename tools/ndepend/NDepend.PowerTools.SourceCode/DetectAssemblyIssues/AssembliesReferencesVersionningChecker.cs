using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.DotNet;

namespace NDepend.PowerTools.DetectAssemblyIssues {
   static class AssembliesReferencesVersionningChecker {
      //
      // Check that assemblies references have the same version than assemblies resolved
      //
      internal static void Go(IEnumerable<IAssemblyInfo> applicationAssembliesInfos, IEnumerable<IAssemblyInfo> thirdPartyAssembliesInfos) {
         Debug.Assert(applicationAssembliesInfos != null);
         Debug.Assert(thirdPartyAssembliesInfos != null);

         var dicoResolvedVersion = new Dictionary<string, Version>(StringComparer.InvariantCultureIgnoreCase);
         foreach (var assemblyInfo in applicationAssembliesInfos.Concat(thirdPartyAssembliesInfos)) {
            var fileName = assemblyInfo.FileNameWithoutExtension.ToLower();
            Debug.Assert(!dicoResolvedVersion.ContainsKey(fileName)); // <- 2 assemblies with same name cannot be resolved
            // Don't check .NET Fx assembly reference version, 
            // because the .NET Fx version resolved is not necessarily the .NET Fx referenced at compile time.
            if (IsDotNetFrameworkAssemblyName(assemblyInfo.FilePath.FileNameWithoutExtension)) { continue; }
           
            dicoResolvedVersion.Add(fileName, assemblyInfo.Version);
         }
         bool noProblem = true;
         foreach (var assemblyInfo in applicationAssembliesInfos) {
            foreach (var assemblyReference in assemblyInfo.AssembliesReferences) {
               Version versionResolved;
               if (!dicoResolvedVersion.TryGetValue(assemblyReference.FileNameWithoutExtension, out versionResolved)) {
                  continue;
               }
               if (versionResolved == assemblyReference.Version) { continue; }
               noProblem = false;
               Console.ForegroundColor = ConsoleColor.Yellow;
               Console.WriteLine(" -> " + assemblyInfo.FileNameWithoutExtension + " references " + assemblyReference.FileNameWithoutExtension + " v" + assemblyReference.Version + " but the resolved version is " + versionResolved);
            }
         }
         if (noProblem) {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("No assembly reference versionning issue found!");
         }
      }

      private static bool IsDotNetFrameworkAssemblyName(string fileNameWithoutExtension) {
         Debug.Assert(fileNameWithoutExtension != null);
         var fileNameWithoutExtensionLower = fileNameWithoutExtension.ToLower();
         return fileNameWithoutExtensionLower == "mscorlib" ||
                fileNameWithoutExtensionLower == "system" ||
                fileNameWithoutExtensionLower.StartsWith("system.") ||
                fileNameWithoutExtensionLower.StartsWith("microsoft.");
                
      }
   }
}
