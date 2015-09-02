using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;


namespace NDepend.PowerTools {
   class Program {



      // TODON PowerTools to do

      //  NDepend.TechnicalDebt  :                 Compute technical debt in hours and days
      //  NDepend.Stable Dependencies Principle Checker  http://oncoding.blogspot.fr/2012/03/stable-dependencies-principle-sdp-and.html
      //  NDepend.PowerTools.LayerValidation    :  do a small layer validation app!  (UI, DAL, Business, MVC...)
      //  NDepend.PowerTools.LawOfDemeter       : shows violation of law of demeters   (problem, we don't have method type parameters!!  http://en.wikipedia.org/wiki/Law_of_Demeter)
      //  NDepend.PowerTools.FabricatedComplexity : dissociate business problem complexity from implementation complexity
      //                                          http://www.headwaysoftware.com/help/dotnet/structure101/index.html?xs_excess-complexity.html
      //                                          http://codebetter.com/patricksmacchia/2009/06/28/fighting-fabricated-complexity/
      //  NDepend.PowerTools.3DCodeCity         : refaire code CodeCity   http://www.inf.usi.ch/phd/wettel/codecity.html  avec WPF
      //  NDepend.PowerTools.ResponseMetric     : could we compute response metric for type or method??
      //                                          Algorithm available here: http://stackoverflow.com/questions/22623479/how-to-measure-rft-metric-in-ndepend
      //  NDepend.PowerTools.AppWordsSpelling   : Detect words mispelled by comparing isolated spelling errors compared to most used spell.
      //  
      //  NDepend.PowerTools.BraithwaiteCorrelation
      //   http://peripateticaxiom.blogspot.com/2006/05/complexity-and-test-first-0.html
      //   http://blog.objectmentor.com/articles/2009/06/08/metrics-of-moment



      // ************************** IMPORTANT ***********************************
      // All programs using NDepend.API.dll should have this type AssemblyResolver
      // parametrized with the relative path to the dir "$NDependInstallDir$\Lib".
      // Since  NDepend.PowerTool.exe  is in the dir "$NDependInstallDir$"
      // the relative path is @".\Lib"
      private static readonly AssemblyResolver s_AssemblyResolver = new AssemblyResolver(@".\Lib");

      [STAThread]
      static void Main() {
         AppDomain.CurrentDomain.AssemblyResolve += s_AssemblyResolver.AssemblyResolveHandler;

         // Call SetProcessDPIAware() to avoid Windows blown-out our UI on high DPI!
         // http://stackoverflow.com/questions/7003316/windows-display-setting-at-150-still-shows-96-dpi
         var b = CallSetProcessDPIAware();
         Debug.Assert(b);

         MainSub();
      }

      // MainSub() is here to avoids that the Main() method uses something
      // from NDepend.API without having registered AssemblyResolveHandler!
      [MethodImpl(MethodImplOptions.NoInlining)]
      static void MainSub() {

         var powerTools = new IPowerTool[] {
                  new CodeQueryConsole.CodeQueryConsolePowerTool(),
                  new DetectAssemblyIssues.DetectAssemblyIssuesPowerTool(),
                  new APIChanges.APIChangesPowerTool(),
                  new SearchForDuplicateCode.SearchForDuplicateCodePowerTool(),
                  new DeadCode.DeadCodePowerTool(),
                  new Evolution.EvolutionPowerTool(),
                  new ReviewMethodChanges.ReviewMethodChangesPowerTool(),
                  new SearchTypesByName.SearchTypesByNamePowerTool(),
                  new AppWords.AppWordsPowerTool(),
                  new AnalyzeCodeOnDisk.AnalyzeCodeOnDiskPowerTool(),
                  new Trend.ListTrendValuesPowerTool(),
                  new Trend.LogTrendValuesPowerTool(),
                  new AnalyzeAssembliesInFolder.AnalyzeAssembliesInFolder(),
                  new AnalyzeDotNetFramework.DotNetFrameworkAnalysisPowerTool(),
                  new CQL2CQLinq.CQL2CQLinqPowerTool(),
                  new DotNetFrameworkCoreAPIChanges.DotNetFrameworkCoreAPIChangesPowerTool(),
         };
         Console.BackgroundColor = ConsoleColor.Black;
         Console.Clear();
         Console.CancelKeyPress += delegate(object sender, ConsoleCancelEventArgs args) {
                                      args.Cancel = true;
                                   };

         while (true) {
            //
            // Show intro
            //
            Console.ForegroundColor = ConsoleColor.White;
            Console.BackgroundColor = ConsoleColor.DarkCyan;
            Console.WriteLine();
            Console.WriteLine("  ***   Welcome to NDepend Power Tools ©  ***   ");
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.BackgroundColor = ConsoleColor.Black;
            Console.WriteLine();
            Console.WriteLine("Power Tools are a set of short programs based on NDepend API.");
            Console.WriteLine("Power Tools aims at demonstrating NDepend API syntax and capabilities.");
            Console.WriteLine();
            Console.WriteLine("Get inspired from power tools source code,");
            Console.WriteLine("and quickly develop any code tool to handle your own needs.");
            Console.WriteLine("This includes code rules checker, code diff reporter,");
            Console.WriteLine("handy development tools, build process checker,");
            Console.WriteLine("unit tests focusing on code quality/diff/structure,");
            Console.WriteLine("code visualization tools...");
            Console.WriteLine("");
            Console.WriteLine("Enjoy!  ");
            Console.WriteLine("");
            Console.ForegroundColor = ConsoleColor.White;

            //
            // Show list of power tools
            //
            var aCharNumber = Convert.ToInt16('a');
            for (var i = 0; i <= powerTools.Length; i++) {
               Console.ForegroundColor = i % 2 == 0 ? ConsoleColor.White : ConsoleColor.Gray;
               string powerToolName = (i == powerTools.Length) ? "Exit" : powerTools[i].Name;
               Console.WriteLine(" " + Convert.ToChar(aCharNumber + i) + ")  " + powerToolName);
            }
            Console.ForegroundColor = ConsoleColor.White;

            //
            // Get power tool user choosen
            //
            int powerToolIndex;
            while (true) {
               Console.Write("Enter power tool key:");
               var keyChar = Char.ToLower(Console.ReadKey().KeyChar);
               Console.WriteLine("");
               powerToolIndex = Convert.ToInt16(keyChar) - aCharNumber;
               if (powerToolIndex < 0 || powerToolIndex > powerTools.Length) {
                  Console.Write(keyChar + " is not a valid tool key!");
                  continue;
               }
               break;
            }

            if (powerToolIndex == powerTools.Length) { break; }
            var powerTool = powerTools[powerToolIndex];

            //
            // Run powerTool in loop
            //
            try {
               // Show power tool description
               Console.WriteLine(""); Console.WriteLine(""); Console.WriteLine("");
               Console.BackgroundColor = ConsoleColor.DarkGreen;
               Console.ForegroundColor = ConsoleColor.White;
               Console.WriteLine("   ***   " + powerTool.Name + "   ***   ");
               Console.BackgroundColor = ConsoleColor.Black;
               Console.ForegroundColor = ConsoleColor.Green;
               foreach (var line in powerTool.Description) {
                  Console.WriteLine(line);
               }
               Console.ForegroundColor = ConsoleColor.White;


               // Run power tool
               Console.WriteLine("");
               powerTool.Run();
            } catch (Exception ex) {
               Console.ForegroundColor = ConsoleColor.Red;
               
               // Determine if it is a licensing exception (hidden as inner inner exception!)!
               var innerEx = ex.InnerException;
               if (innerEx != null && ex is TypeInitializationException && innerEx is TargetInvocationException) {
                  var innerInnerEx = innerEx.InnerException;
                  if (innerInnerEx != null) {
                     Console.WriteLine("Inner Inner Exception Message: {" + innerInnerEx.Message + "}");
                     goto LICENSING_EXCEPTION_SHOWN;
                  }
               }

               Console.WriteLine("Exception thrown by power tool {" + powerTool.Name + "}");
               Console.WriteLine("Exception Type: {" + ex.GetType().Namespace + "." + ex.GetType().Name + "}");
               Console.WriteLine("Exception Message: {" + ex.Message + "}");

               if (innerEx != null) {
                  Console.WriteLine("   Inner Exception Type: {" + innerEx.GetType().Namespace + "." + innerEx.GetType().Name + "}");
                  Console.WriteLine("   Inner Exception Message: {" + innerEx.Message + "}");
               }

LICENSING_EXCEPTION_SHOWN:
               Console.ForegroundColor = ConsoleColor.White;
            }
            Console.CursorLeft = 0;
            ConsoleUtils.ShowNLinesOnConsole(5, ConsoleColor.Black);
            Console.CursorTop -= 2;
            Console.ForegroundColor = ConsoleColor.White;
            Console.BackgroundColor = ConsoleColor.DarkCyan;
            Console.WriteLine("Press a key to run another power tool!");
            Console.ReadKey();
            Console.BackgroundColor = ConsoleColor.Black;
            Console.Clear();
         } // Next power tool ...
      }





      #region SetProcessDPIAware
      //------------------------------------------------------------
      //
      //  SetProcessDPIAware
      //
      //------------------------------------------------------------

      [DllImport("user32.dll", SetLastError = true)]
      private static extern bool SetProcessDPIAware();

      public static bool CallSetProcessDPIAware() {
         //
         // Calling SetProcessDPIAware : This is a new API in Vista's user32.dll
         //
         var osMajorVersion = Environment.OSVersion.Version.Major;
         if (osMajorVersion <= 5) {
            // SetProcessDPIAware() is not supported on Windows XP!
            // SetProcessDPIAware() is supported since Windows Vista!
            // http://www.rw-designer.com/DPI-aware   
            return true;
         }

         try {
            //  We MUST call SetProcessDPIAware() to avoid Blown Away VisualNdepend on 150%+ DPI!!!
            //  This applies to NDepend dialog shown through the API, and also own user dialogs!
            //
            // Explanation here:    http://stackoverflow.com/questions/7003316/windows-display-setting-at-150-still-shows-96-dpi
            // On Windows Vista and Windows 7, with DPIs above 120 (I think) and applications that it considers to be non-DPI aware, they switch into DPI Virtualization Mode.
            // This does exactly what you're seeing - the application thinks it's running in 96dpi while Windows blows it up and renders everything bigger (and blurrier).
            // http://msdn.microsoft.com/en-us/library/dd464660(VS.85).aspx#declaring_dpi_awareness
            return SetProcessDPIAware();
         } catch {
            return true;
         }
      }
      #endregion SetProcessDPIAware
   }
}
