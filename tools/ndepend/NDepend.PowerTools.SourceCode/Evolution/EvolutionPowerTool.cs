using System;
using System.Linq;
using NDepend.Helpers;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Analysis;
using NDepend.Project;


namespace NDepend.PowerTools.Evolution {
   class EvolutionPowerTool : IPowerTool {
      public string Name {
         get { return "Code Base Evolution"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "Retrieves the multiple analysis results available for a project...",
                  "...and display the evolution of a few application metric.",
                  "It would be easy to process some graphical charts with these metrics evolution...",
                  "...or to follow the evolution of one or several particular code elements...",
                  "...or to detect portions of the code that keeps being refactored again and again."
               };
         }
      }



      public void Run() {
         var projectManager = new NDependServicesProvider().ProjectManager;
         IProject project;
         if (!projectManager.ShowDialogChooseAnExistingProject(ConsoleUtils.MainWindowHandle, out project)) { return; }

         // Fetch analysis results reference for the choosen project
         var analysisResultRefs = project.GetAvailableAnalysisResultsRefs();

         // Need at least 2 analysis results
         if (analysisResultRefs.Count < 2) {
            Console.BackgroundColor = ConsoleColor.Black;
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Cannot find at least 2 historic analysis results for the project {" + project.Properties.Name+ "}");
            Console.WriteLine("To keep historic analysis results, please configure properly: Project Properties > Analysis > Historic Analysis Results > Settings");
            return;
         }


         bool showAll = false;
         Console.BackgroundColor = ConsoleColor.Black;
         Console.ForegroundColor = ConsoleColor.White;

         // Sort by analysis date ascending
         analysisResultRefs = analysisResultRefs.OrderBy(analysisResultRef => analysisResultRef.Date).ToReadOnlyClonedList();

// TODO --> use .NET 4 System.Windows.Forms.DataVisualization.dll (if available), to plot chart??

         //
         // For each analysisResultRef found for this project
         //
         foreach (var analysisResultRef in analysisResultRefs) {
            var analysisResult = ProjectAnalysisUtils.LoadAnalysisShowProgressOnConsole(analysisResultRef);

            Console.WriteLine();
            Console.WriteLine("Analysis result produced on " + analysisResultRef.Date);

            //
            // -->If you wish to follow the evolution of a particular code element (a type for example)
            //    just retrieve the IType in each code base by type.FullName
            //      var type = codeBase.Types.WithFullName("Namespace.TypeName").
            //
            //    You can also build a ICompareContext for each 2 consecutive analysis result loaded.
            //      analysisResultNext.CodeBase.CreateCompareContextWithOlder(analysisResultPrevious.CodeBase)
            //    and use the methods
            //      ICompareContext.OlderVersion(ICodeElement):ICodeElement
            //      ICompareContext.NewerVersion(ICodeElement):ICodeElement
            //    to retrieve the newer versions of the particular code element, from its older version.
            ProjectAnalysisUtils.ShowConsoleAnalysisResult(analysisResult.CodeBase);

            // Ask all/next/quit?
            if (showAll) { continue; }
            switch (ConsoleUtils.ConsoleAskShowNextAllStop()) {
               case AllNextQuit.Next:
                  continue;
               case AllNextQuit.All:
                  showAll = true;
                  continue;
               default:
                  // Quit
                  return;
            }
         }

      }

   }
}
