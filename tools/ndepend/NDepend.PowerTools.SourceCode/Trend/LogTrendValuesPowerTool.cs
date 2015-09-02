using System;
using System.Diagnostics;
using NDepend.Analysis;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;

namespace NDepend.PowerTools.Trend {

   class LogTrendValuesPowerTool : IPowerTool {

      public string Name {
         get { return "Dated Log Trend Values"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "First analysis a project, obtain an analysis result,",
                  "then compute Trend Values from the analysis result",
                  "and log them, with any past or future date.",
                  "This power tool aims at showing how to use NDepend.API",
                  "to persist programatically trend values.",
               };
         }
      }




      public void Run() {
         var projectManager = new NDependServicesProvider().ProjectManager;
         IProject project;
         if (!projectManager.ShowDialogChooseAnExistingProject(ConsoleUtils.MainWindowHandle, out project)) {
            return;
         }
         Debug.Assert(project != null);

         var analysisResult = ProjectAnalysisUtils.RunAnalysisShowProgressOnConsole(project);
         Console.WriteLine("Begin Compute and Store Trend Metrics Values");

         // LogTrendMetrics() is an extension method declared by NDepend.Analysis.ExtensionMethodsProjectAnalysis
         // Two overloads are available:
         //   void LogTrendMetrics(this IAnalysisResult analysisResult, DateTime logDateTime)
         //   void LogTrendMetrics(this IAnalysisResult analysisResult, ICompareContext compareContext, DateTime logDateTime)
         analysisResult.LogTrendMetrics(DateTime.Now);  // <- Here any past or future date can be used!

         Console.WriteLine("End Compute and Store Trend Metrics Values");

         

         // 
         //   Also, from trendStore, any metric value can be persisted, with any date.
         //
         //var trendStore = project.Trend.TrendStoreDirectory.GetTrendStoreFromDirectory();
         //trendStore.SaveMetricValues(DateTime.Now, new [] {
         //   new MetricValue(new Metric("Metric1", "Unit1"), 1.0f),
         //   new MetricValue(new Metric("Metric2", "Unit2"), 2.0f),
         //});
         //
         //   The method RemoveMetricValues() can also be used to remove all metric values logged at a certain date.
         //   The date equality is defined by the same dated second, no matter the number of miliseconds.
         //trendStore.RemoveMetricValues(DateTime.Now);
      }
   }
}
