using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows.Forms;
using NDepend.Analysis;
using NDepend.CodeModel;
using NDepend.Path;
using NDepend.Project;
using NDepend.DotNet.VisualStudio;

namespace NDepend.PowerTools.SharedUtils {
   static class ProjectAnalysisUtils {


      internal static IAnalysisResult RunAnalysisShowProgressOnConsole(IProject project) {
         Debug.Assert(project != null);
         var cursorTop = Console.CursorTop;
         Console.CursorVisible = false;
         var stopwatch = new Stopwatch();
         stopwatch.Start();
         var analysisResult = project.RunAnalysis( // AndBuildReport eventually
            analysisLog => { },
            progressLog => ShowProgressDescriptionAndProgressBar(progressLog, cursorTop));
         stopwatch.Stop();
         EraseProgressLogTrace(cursorTop);
         Console.WriteLine("Analysis duration:" + stopwatch.Elapsed.ToString());
         Console.CursorVisible = true;
         return analysisResult;
      }


      internal static IAnalysisResult LoadAnalysisShowProgressOnConsole(IAnalysisResultRef analysisResultRef) {
         Debug.Assert(analysisResultRef != null);
         var cursorTop = Console.CursorTop;
         Console.CursorVisible = false;
         var analysisResult = analysisResultRef.Load(progressLog => ShowProgressDescriptionAndProgressBar(progressLog, cursorTop));
         EraseProgressLogTrace(cursorTop);
         Console.CursorVisible = true;
         return analysisResult;
      }


      private static void ShowProgressDescriptionAndProgressBar(IProgressLog progressLog, int cursorTop) {
         Debug.Assert(progressLog != null);
         Console.CursorTop = cursorTop;
         var windowWidth = Console.WindowWidth - 1 /* -1 to avoid shaking effect when Console has a small width */;

         // Show progress description
         Console.ForegroundColor = ConsoleColor.White;
         Console.BackgroundColor = ConsoleColor.Black;
         Console.CursorTop = cursorTop;
         Console.CursorLeft = 0;

         var description = progressLog.EstimatedPercentageDone + "% " + progressLog.Description;
         if (description.Length > windowWidth) { description = description.Substring(0, windowWidth); }
         Console.Write(description);
         if (description.Length < windowWidth) {
            Console.Write(new string(' ', windowWidth - description.Length));
         }

         // Show progress bar
         Console.CursorTop = cursorTop + 1;
         Console.CursorLeft = 0;
         var progressBarWidth = progressLog.EstimatedPercentageDone * windowWidth / 100;
         Console.BackgroundColor = ConsoleColor.Green;
         Console.Write(new string(' ', progressBarWidth));
         Console.BackgroundColor = ConsoleColor.DarkGray;
         Console.Write(new string(' ', windowWidth - progressBarWidth));
         Console.BackgroundColor = ConsoleColor.Black;

         Console.CursorTop = cursorTop;
      }


      private static void EraseProgressLogTrace(int cursorTop) {
         Console.CursorTop = cursorTop;
         Console.CursorLeft = 0;
         ConsoleUtils.ShowNLinesOnConsole(4, ConsoleColor.Black);
         Console.CursorTop = cursorTop;
         Console.CursorLeft = 0;
      }



      internal static void ShowConsoleAnalysisResult(ICodeBase codeBase) {
         Debug.Assert(codeBase != null);
         Console.ForegroundColor = ConsoleColor.White;
         Console.WriteLine("  # Application Assemblies " + codeBase.Application.Assemblies.Count().Format1000());
         Console.WriteLine("  # Third-Party Assemblies " + codeBase.ThirdParty.Assemblies.Count().Format1000());
         Console.WriteLine("  # Application Namespaces " + codeBase.Application.Namespaces.Count().Format1000());
         Console.WriteLine("  # Third-Party Namespaces " + codeBase.ThirdParty.Namespaces.Count().Format1000());
         ShowTypeCount("  # Types ", codeBase, t => true);
         Console.WriteLine("  # Methods " + codeBase.Methods.Count().Format1000());
         Console.WriteLine("  # Fields " + codeBase.Fields.Count().Format1000());

         Console.WriteLine("  # Line of Codes " + codeBase.Assemblies.Sum(asm => asm.NbLinesOfCode).Format1000());
         if (codeBase.Application.Assemblies.Where(a => a.NbLinesOfCode == null).Count() > 0) {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Under estimated # Lines of Code (LoC). Some assemblies PDB cannot be found and thus LoC cannot be computed for them.");
            Console.ForegroundColor = ConsoleColor.White;
         }

         Console.WriteLine("  # IL instructions " + codeBase.Assemblies.Sum(asm => asm.NbILInstructions).Format1000());
         ShowTypeCount("  # Classes ", codeBase, t => t.IsClass);
         ShowTypeCount("  # Structures ", codeBase, t => t.IsStructure);
         ShowTypeCount("  # Interfaces ", codeBase, t => t.IsInterface);
         ShowTypeCount("  # Enumerations ", codeBase, t => t.IsEnumeration);
         ShowTypeCount("  # Delegate classes ", codeBase, t => t.IsDelegate);
         ShowTypeCount("  # Exception classes ", codeBase, t => t.IsExceptionClass);
         ShowTypeCount("  # Attribute classes ", codeBase, t => t.IsAttributeClass);
      }
      private static void ShowTypeCount(string metricName, ICodeBase codeBase, Predicate<IType> predicate) {
         Debug.Assert(!string.IsNullOrEmpty(metricName));
         Debug.Assert(codeBase != null);
         Debug.Assert(predicate != null);
         var applicationTypes = codeBase.Application.Types;
         Console.WriteLine(metricName + applicationTypes.Where(t => predicate(t)).Count().Format1000() + "     public " + applicationTypes.Where(t => t.IsPublic && predicate(t)).Count().Format1000());
      }


      internal static bool TryChooseProject(out IProject project) {

         var ndependServicesProvider = new NDependServicesProvider();
         var projectManager = ndependServicesProvider.ProjectManager;
         var visualStudioManager = ndependServicesProvider.VisualStudioManager;

CHOOSE_PROJECT:
         var top = Console.CursorTop;
         Console.CursorLeft = 0;
         
         Console.WriteLine("Please choose...");
         Console.WriteLine("  a) an existing NDepend project");
         Console.WriteLine("  b) one or several Visual Studio solutions to analyze");
         Console.WriteLine("  c) one or several .NET assemblies to analyze");
         Console.WriteLine("");
         var c = Char.ToLower(Console.ReadKey().KeyChar);
         Console.WriteLine();

         switch (c) {
            case 'a':
               if (!projectManager.ShowDialogChooseAnExistingProject(ConsoleUtils.MainWindowHandle, out project)) { goto TRY_AGAIN; }
               break;
            case 'b': {
                  ICollection<IAbsoluteFilePath> solutions;
                  if (!visualStudioManager.ShowDialogSelectVisualStudioSolutionsOrProjects(ConsoleUtils.MainWindowHandle, out solutions)) { goto TRY_AGAIN; }
                  var assemblies = new List<IAbsoluteFilePath>();
                  foreach (var solution in solutions) {
                     assemblies.AddRange(visualStudioManager.GetAssembliesFromVisualStudioSolutionOrProject(solution));
                  }
                  project = projectManager.CreateTemporaryProject(assemblies, TemporaryProjectMode.Temporary);
                  break;
               }
            case 'c': {
                  ICollection<IAbsoluteFilePath> assemblies;
                  if (!projectManager.ShowDialogSelectAssemblies(ConsoleUtils.MainWindowHandle, out assemblies)) { goto TRY_AGAIN; }
                  project = projectManager.CreateTemporaryProject(assemblies, TemporaryProjectMode.Temporary);
                  break;
               }

            case (char)Keys.Escape:  // ESC to exit!
               project = null;
               return false;

            default:
         TRY_AGAIN:
               var nbLinesToErase = Console.CursorTop - top;
               Console.CursorTop = top;
               Console.CursorLeft = 0;
               ConsoleUtils.ShowNLinesOnConsole(nbLinesToErase, ConsoleColor.Black);
               Console.WriteLine("(ESC to exit)");
               Console.CursorTop = top;
               Console.CursorLeft = 0;
               goto CHOOSE_PROJECT;
         }
         Debug.Assert(project != null);




         Console.ForegroundColor = ConsoleColor.DarkGray;
         Console.Write("Project selected: ");
         Console.ForegroundColor = ConsoleColor.White;
         Console.WriteLine(project.Properties.Name);
         Console.WriteLine();
         return true;
      }


      internal static bool TryChooseAnalysisResult(out IAnalysisResult analysisResult) {
         IProject project;
         if (!TryChooseProject(out project)) {
            analysisResult = null;
            return false;
         }
         Debug.Assert(project != null);

         IAnalysisResultRef analysisResultRef;
         if (project.TryGetMostRecentAnalysisResultRef(out analysisResultRef)) {
            // Most recent analysis result
            analysisResult = LoadAnalysisShowProgressOnConsole(analysisResultRef);
            return true;
         }
         // No analysis result available => Run analysis to obtain one
         analysisResult = RunAnalysisShowProgressOnConsole(project);
         return true;
      }

      internal static bool TryGetAssembliesToCompareAndAnalyzeThem(out ICompareContext compareContext) {
         var analysisManager = new NDependServicesProvider().AnalysisManager;

         var top = Console.CursorTop;
         Console.CursorLeft = 0;

         IProject projectOlder, projectNewer;
         IAnalysisResultRef analysisResultRefOlder, analysisResultRefNewer;
         
         Console.WriteLine("Please choose older and newer versions of the code base...");

         bool dialogOk = analysisManager.ShowDialogBuildComparison(
            ConsoleUtils.MainWindowHandle, 
            out projectOlder, 
            out analysisResultRefOlder, 
            out projectNewer, 
            out analysisResultRefNewer);
         if (!dialogOk) {
            compareContext = null;
            return false;
         }


         var nbLinesToErase = Console.CursorTop - top;
         Console.CursorTop = top;
         Console.CursorLeft = 0;
         ConsoleUtils.ShowNLinesOnConsole(nbLinesToErase, ConsoleColor.Black);
         Console.CursorTop = top;
         Console.CursorLeft = 0;
         

         //
         // Load or analyze
         //
         IAnalysisResult analysisResultOlder, analysisResultNewer;
         if (analysisResultRefOlder == null) {
            Debug.Assert(projectOlder != null);
            Console.WriteLine("Analyze older version of assemblies");
            analysisResultOlder = RunAnalysisShowProgressOnConsole(projectOlder);
         } else {
            Console.WriteLine("Load older analysis result");
            analysisResultOlder = LoadAnalysisShowProgressOnConsole(analysisResultRefOlder);
         }

         if (analysisResultRefNewer == null) {
            Debug.Assert(projectNewer != null);
            Console.WriteLine("Analyze newer version of assemblies");
            analysisResultNewer = RunAnalysisShowProgressOnConsole(projectNewer);
         } else {
            Console.WriteLine("Load newer analysis result");
            analysisResultNewer = LoadAnalysisShowProgressOnConsole(analysisResultRefNewer);
         }

         //
         // Re-erase
         //
         var nbLinesToErase2 = Console.CursorTop - top;
         Console.CursorTop = top;
         Console.CursorLeft = 0;
         ConsoleUtils.ShowNLinesOnConsole(nbLinesToErase2, ConsoleColor.Black);
         Console.CursorTop = top;
         Console.CursorLeft = 0;


         //
         // Show compare description
         //
         Console.ForegroundColor = ConsoleColor.DarkGray;
         Console.Write("Comparing: ");
         Console.ForegroundColor = ConsoleColor.White;
         ShowAnalysisResultRefDescription(analysisResultOlder.AnalysisResultRef);
         Console.WriteLine();

         Console.ForegroundColor = ConsoleColor.DarkGray;
         Console.Write("     with: ");
         Console.ForegroundColor = ConsoleColor.White;
         ShowAnalysisResultRefDescription(analysisResultNewer.AnalysisResultRef);

         compareContext = analysisResultNewer.CodeBase.CreateCompareContextWithOlder(analysisResultOlder.CodeBase);
         return true;
      }

      private static void ShowAnalysisResultRefDescription(IAnalysisResultRef analysisResultRef) {
         Debug.Assert(analysisResultRef != null);
         Console.Write("Project ");
         Console.Write(analysisResultRef.Project.Properties.Name);
         Console.Write("    analysis done at " + analysisResultRef.Date.ToString());
      }




      internal static bool TryGetCompareContextDefinedByBaseline(IAnalysisResult analysisResult, out ICompareContext compareContext) {
         Debug.Assert(analysisResult != null);
         var project = analysisResult.AnalysisResultRef.Project;
         IAnalysisResultRef analysisResultUsedInUIRef;
         if (project.BaselineInUI.TryGetAnalysisResultRefToCompareWith(out analysisResultUsedInUIRef) == TryGetAnalysisResultRefToCompareWithResult.DoCompareWith) {
            Debug.Assert(analysisResultUsedInUIRef != null);
            compareContext = LoadCompareContext(analysisResultUsedInUIRef, analysisResult);
            return true;
         }

         IAnalysisResultRef analysisResultUsedDuringAnalysisRef;
         if (project.BaselineDuringAnalysis.TryGetAnalysisResultRefToCompareWith(out analysisResultUsedDuringAnalysisRef) == TryGetAnalysisResultRefToCompareWithResult.DoCompareWith) {
            Debug.Assert(analysisResultUsedDuringAnalysisRef != null);
            compareContext = LoadCompareContext(analysisResultUsedDuringAnalysisRef, analysisResult);
            return true;
         }

         compareContext = null;
         return false;
      }

      private static ICompareContext LoadCompareContext(IAnalysisResultRef analysisResultRef, IAnalysisResult analysisResult) {
         Debug.Assert(analysisResultRef != null);
         Debug.Assert(analysisResult != null);
         var otherAnalysisResult = LoadAnalysisShowProgressOnConsole(analysisResultRef);
         return analysisResult.CodeBase.CreateCompareContextWithOlder(otherAnalysisResult.CodeBase);
      }
   }
}
