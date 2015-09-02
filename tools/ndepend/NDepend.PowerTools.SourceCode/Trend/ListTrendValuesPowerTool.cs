using System;
using System.Linq;
using System.Text;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;
using NDepend.Trend;

namespace NDepend.PowerTools.Trend {

   class ListTrendValuesPowerTool : IPowerTool {

      public string Name {
         get { return "List stored Trend Values"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "List stored Trend Values for a project.",
                  "This power tool aims at showing how to use NDepend.API",
                  "to access programatically stored trend values.",
               };
         }
      }




      public void Run() {
         var projectManager = new NDependServicesProvider().ProjectManager;
         IProject project;
         if (!projectManager.ShowDialogChooseAnExistingProject(ConsoleUtils.MainWindowHandle, out project)) {
            return;
         }

         // Get the trend store object
         var trendStore = project.Trend.TrendStoreDirectory.GetTrendStoreFromDirectory();
         var years = trendStore.GetYearsContainingValuesDescending();
         if (!years.Any()) {
            Console.WriteLine("No trend values stored for the project {" + project.Properties.Name + "}.");
            return;
         }

         var lastYear = years.Last();
         var yearsStringBuilder = new StringBuilder();
         foreach (var year in years) {
            yearsStringBuilder.Append(year.ToString());
            if (lastYear != year) { yearsStringBuilder.Append(", "); }
         }
         Console.WriteLine("The project {" + project.Properties.Name + "} has trend values stored for years " + yearsStringBuilder.ToString());
         
         // Load all values in one call
         var historyData = trendStore.Load(years);
         var metricHistory = historyData.MetricsHistories.First().Value; // Only list the trend values for the first metric
         Console.WriteLine("Trend values for the metric {" + metricHistory.Metric.Name + "}");

         var unit = metricHistory.Metric.Unit;
         foreach (var datedValue in metricHistory.DatedValuesDescending) {
            Console.WriteLine(datedValue.Date.ToString() + "  " + datedValue.Value.ToString() + " " + unit);
         }
      }
   }
}
