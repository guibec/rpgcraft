using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.DotNet;

namespace NDepend.PowerTools.DetectAssemblyIssues {
   static class CoherencyChecker {

      internal static void Go<T>(IEnumerable<IAssemblyInfo> applicationAssembliesInfos, Func<IAssemblyInfo, T> getPropertyProc, string propertyDesc) {
         Debug.Assert(applicationAssembliesInfos != null);
         T commonValue = default(T);
         if (!TryGetPropertyValue(
                  applicationAssembliesInfos,
                  getPropertyProc,
                  propertyDesc,
                  out commonValue)) {
            return;
         }

         var assembliesWithoutVersion = from assemblyInfo in applicationAssembliesInfos
                                        where !getPropertyProc(assemblyInfo).Equals(commonValue)
                                        select assemblyInfo;
         var count = assembliesWithoutVersion.Count();
         if (count == 0) {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("All assemblies have the same " + propertyDesc + "  " + commonValue);
            return;
         }


         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine(count + " " + (count == 1 ? "assembly deosn't " : "assemblies don't ") + "have the " + propertyDesc + " " + commonValue);
         foreach (var assemblyInfo in assembliesWithoutVersion) {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.Write(" -> " + assemblyInfo.FileNameWithoutExtension + "   (" + propertyDesc + " " + getPropertyProc(assemblyInfo) + ")");
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine("   " + assemblyInfo.FilePath.ToString());
         }
      }


      private static bool TryGetPropertyValue<T>(IEnumerable<IAssemblyInfo> assembliesInfos, Func<IAssemblyInfo, T> getPropertyProc, string propertyDesc, out T value) {
         Debug.Assert(assembliesInfos != null);
         Debug.Assert(getPropertyProc != null);
         Debug.Assert(!string.IsNullOrEmpty(propertyDesc));
         Debug.Assert(assembliesInfos.Count() >= 3);

         var dicoValues = new Dictionary<T, int>();
         foreach (var assemblyInfo in assembliesInfos) {
            var assemblyVersion = getPropertyProc(assemblyInfo);
            if (!dicoValues.ContainsKey(assemblyVersion)) {
               dicoValues.Add(assemblyVersion, 1);
               continue;
            }
            dicoValues[assemblyVersion]++;
         }

         KeyValuePair<T, int>? mostUsedVersion = null;
         foreach (var pair in dicoValues) {
            if (mostUsedVersion == null) {
               mostUsedVersion = pair;
               continue;
            }
            if (pair.Value > mostUsedVersion.Value.Value) {
               mostUsedVersion = pair;
            }
         }
         Debug.Assert(mostUsedVersion != null);

         var percent = (mostUsedVersion.Value.Value * 100 / assembliesInfos.Count());
         if (percent < 50) {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine(propertyDesc + " cannot be infered. There are no 50% of the assemblies sharing the same value");
            value = default(T);
            return false;
         }

         value = mostUsedVersion.Value.Key;
         Console.ForegroundColor = ConsoleColor.White;
         Console.WriteLine("The common value for " + propertyDesc + " is " + value + ". " + percent + "% of the assemblies share this value.");
         return true;
      }

   }
}
