using System.Reflection;
using System.Runtime.CompilerServices;


namespace NDepend {

   //
   // This code is based on the fact that:
   //   - The class System.Linq.Enumerable appeared with .NET v3.5
   //   - This class is declared in the assembly System.Core.dll that also appeared with .NET v3.5
   //   - The exception FileNotFoundException is thrown when an assembly is no found.
   //
   static class DotNetFx35Checker {
      [MethodImpl(MethodImplOptions.NoInlining)]
      internal static bool IsDotNetFx35Available(out string failureReason, out string productName) {
         productName = "NDepend";
         try {
            TestLinqAvailable();
            failureReason = null;
            return true;
         } catch (System.IO.FileNotFoundException) {
            var productVersion = Assembly.GetExecutingAssembly().GetName().Version;
            var productNameAndVersion = productName + " v" + productVersion.Major + "." + productVersion.Minor;
            failureReason = "To run " + productNameAndVersion + ", please download and install .NET Fx v3.5 or upper version.";
            return false;
         }
      }
      [MethodImpl(MethodImplOptions.NoInlining)]
      private static void TestLinqAvailable() {
         var i = System.Linq.Enumerable.Count(new int[] { 1 });
      }
   }
}
