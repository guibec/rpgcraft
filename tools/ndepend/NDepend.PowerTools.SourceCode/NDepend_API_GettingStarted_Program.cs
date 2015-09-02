using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.Analysis;
using NDepend.CodeModel;
using NDepend.Path;
using NDepend.Project;

namespace NDepend {


   //
   // This class code is here to help Getting Started with NDepend.API
   // by showing the relations between the interfaces IProject, IAnalysisResult and ICodeBase.
   //
   class NDepend_API_GettingStarted_Program {

      // Needs an STA thread to shows dialog
      [STAThread] 
      static void Main() {
         //-------------------------------------------------------------------------------------
         //
         // 0) Creates a NDependServicesProvider object
         //
         var ndependServicesProvider = new NDependServicesProvider();

         //-------------------------------------------------------------------------------------
         //
         // 1) obtain some VS solution or project file path
         //
         var visualStudioManager = ndependServicesProvider.VisualStudioManager;
         ICollection<IAbsoluteFilePath> vsSlnOrProjFilePaths;
         visualStudioManager.ShowDialogSelectVisualStudioSolutionsOrProjects(Process.GetCurrentProcess().MainWindowHandle, out vsSlnOrProjFilePaths);
         // Could also use:  visualStudioManager.GetMostRecentlyUsedVisualStudioSolutionOrProject() can also be used

         //-------------------------------------------------------------------------------------
         //
         // 2) obtains assemblies file path to analyze
         //
         var assembliesFilePath = (from vsSlnOrProjFilePath in vsSlnOrProjFilePaths
                                   from assembliesFilePathTmp in visualStudioManager.GetAssembliesFromVisualStudioSolutionOrProject(vsSlnOrProjFilePath)
                                   select assembliesFilePathTmp).Distinct().ToArray();

         //-------------------------------------------------------------------------------------
         //
         // 3) gets or create a IProject object
         //
         var projectManager = ndependServicesProvider.ProjectManager;
         var project = projectManager.CreateTemporaryProject(assembliesFilePath, TemporaryProjectMode.Temporary);
         // Or, to get a IProject object, could also use 
         //   projectManager.CreateBlankProject()  to create a persisten project 
         //   and then project.CodeToAnalyze.SetApplicationAssemblies() 
         //   and then projectManager.SaveProject(project); o save the project file
         //
         // Or, to get an existing IProject object, could also use
         //    projectManager.ShowDialogChooseAnExistingProject(out project)
         // Or programmatically list most recently used NDepend projects on this machine through
         //    projectManager.GetMostRecentlyUsedProjects()


         //-------------------------------------------------------------------------------------
         //
         // 4) gets an IAnalysisResult object from the IProject object
         //
         var analysisResult = project.RunAnalysis();  // *** This particular method works only with a Build Machine license ***
         //  Or  project.RunAnalysisAndBuildReport()  // *** This particular method works only with a Build Machine license ***

         // Or, to get a IAnalysisResult object, first gets a IAnalysisResultRef object, that represents a reference to a persisted IAnalysisResult object
         //    project.TryGetMostRecentAnalysisResultRef() or project.GetAvailableAnalysisResultsRefs() or project.GetAvailableAnalysisResultsRefsGroupedPerMonth()
         // and then analysisResultRef.Load()


         //-------------------------------------------------------------------------------------
         //
         // 5) gets a ICodeBase object from the IAnalysisResult object
         //
         var codeBase = analysisResult.CodeBase;
         // Or eventually a ICompareContext object if you wish to analyze diff 
         // codeBase.CreateCompareContextWithOlder(olderCodeBase) 


         //-------------------------------------------------------------------------------------
         //
         // 6) use the code model API to query code and do develop any algorithm you need!
         // 
         // For example here we are looking for complex methods
         var complexMethods = (from m in codeBase.Application.Methods 
                               where m.ILCyclomaticComplexity > 10 
                               orderby m.ILCyclomaticComplexity descending 
                               select m).ToArray();
         if (complexMethods.Length == 0) { return; }
         Console.WriteLine("Press a key to show the " + complexMethods.Length + " most complex methods");
         Console.ReadKey();
         foreach (var m in complexMethods) {
            Console.WriteLine(m.FullName + " has a IL cyclomatic complexity of " + m.ILCyclomaticComplexity);
         }

         //-------------------------------------------------------------------------------------
         //
         // 7) eventually lets the user opens source file declaration
         //
         if (complexMethods.First().SourceFileDeclAvailable) {
            var mostComplexMethod = complexMethods.First();
            Console.WriteLine("Press a key to open the source code decl of the most complex method?");
            Console.ReadKey();
            mostComplexMethod.TryOpenSource();
            // Eventually use ExtensionMethodsTooling.TryCompareSourceWith(NDepend.CodeModel.ISourceFileLine,NDepend.CodeModel.ISourceFileLine)
            // to compare 2 different versions of a code element
         }

      }



      public static void Go() {

         var projectManager = new NDependServicesProvider().ProjectManager;
         var project = projectManager.LoadProject(@"C:\Dir\NDependProject.ndproj".ToAbsoluteFilePath());

         Console.WriteLine("Load newer analysis result");
         IAnalysisResultRef mostRecentAnalysisResultRef;
         if (!project.TryGetMostRecentAnalysisResultRef(out mostRecentAnalysisResultRef)) {
            Console.WriteLine("Can't find most recent analysis result for project " + project.Properties.FilePath.ToString());
            return;
         }
         var mostRecentAnalysisResult = mostRecentAnalysisResultRef.Load();

         Console.WriteLine("load old analysis result");
         IAnalysisResultRef analysisResultToCompareWithRef;
         if (project.BaselineInUI.TryGetAnalysisResultRefToCompareWith(out analysisResultToCompareWithRef) != TryGetAnalysisResultRefToCompareWithResult.DoCompareWith) {
            Console.WriteLine("Can't find baseline UI analysis to compare with");
            return;
         }
         var analysisResultToCompareWith = analysisResultToCompareWithRef.Load();

         Console.WriteLine("Create compare context");
         var compareContext = mostRecentAnalysisResult.CodeBase.CreateCompareContextWithOlder(analysisResultToCompareWith.CodeBase);

         foreach (var m in compareContext.NewerCodeBase.Namespaces.Where(compareContext.WasAdded)) {
            Console.WriteLine(m.Name);
         }
      }
   }
}
