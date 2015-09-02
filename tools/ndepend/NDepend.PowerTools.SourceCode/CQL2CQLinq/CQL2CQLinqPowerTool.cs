
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using NDepend.Analysis;
using NDepend.CodeQuery;
using NDepend.Path;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;

namespace NDepend.PowerTools.CQL2CQLinq {
   class CQL2CQLinqPowerTool : IPowerTool {
      public string Name {
         get { return "Convert CQL queries/rules of a project to CQLinq queries/rules"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "For a project file named {XYZ.ndproj} a backup file named {XYZ.bkp#.ndproj} is created before the conversion.",
               };
         }
      }

      public void Run() {
         IAnalysisResult analysisResult;
         if (!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);

         var project = analysisResult.AnalysisResultRef.Project;
         CreateBackupProjectFile(project);

         //
         // Convert all CQL queries compiled successfully!
         //
         var queriesSet = project.CodeQueries.CodeQueriesSet;
         var queriesController = queriesSet.Controller;
         var allQueries = queriesSet.AllQueriesRecursive;
         var allQueriesString = allQueries.Select(q => q.QueryString).ToArray();
         var allQueriesCompiled = allQueriesString.CompileMany(analysisResult.CodeBase);

         // Do the conversion and build an association dictionary, CQL to CQLinq queryString
         var dicoCQLToCQLinq = new Dictionary<string, string>();
         foreach (var queryCompiled in allQueriesCompiled) {
            if (queryCompiled.HasErrors) { continue; }
            if (queryCompiled.KindOfCodeQuerySyntax != KindOfCodeQuerySyntax.CQL) { continue; }
            if (dicoCQLToCQLinq.ContainsKey(queryCompiled.OriginalQueryString)) { continue; }

            var queryCompiledSuccess = queryCompiled.QueryCompiledSuccess;
            Debug.Assert(queryCompiledSuccess != null);
            var cqlinqQueryString = queryCompiledSuccess.ConvertToCQLinq(); // <-- The conversion is performed here!
            dicoCQLToCQLinq.Add(queryCompiledSuccess.OriginalQueryString, cqlinqQueryString);
         }

         // Update CQL queries converted to CQLinq
         int nbQueriesConverted = 0;
         foreach (var pair in dicoCQLToCQLinq) {
            var queryCQL = pair.Key;
            var queryCQLinq = pair.Value;
            foreach (var query in queriesController.RootParent.AllQueriesRecursive.ToList()) {
               Debug.Assert(query.RootParent == queriesController.RootParent);
               if(query.QueryString != queryCQL) { continue; }
               var newQuery = queriesController.CreateQuery(
                  query.IsActive,
                        queryCQLinq,
                  query.DisplayStatInReport,
                  query.DisplayListInReport,
                  query.DisplaySelectionViewInReport,
                  query.IsCriticalRule);
               nbQueriesConverted++;
               queriesController.DoUpdateQueryObject(query, newQuery);
               break;
            }
         }

         var groupToSave = queriesController.RootParent.ChildGroups.Single();

         // This call provokes saving queries in project file
         project.CodeQueries.CodeQueriesSet = groupToSave;

         Console.WriteLine(nbQueriesConverted + " queries converted!");
      }

      private static void CreateBackupProjectFile(IProject project) {
         Debug.Assert(project != null);
         var projectFilePath = project.Properties.FilePath;
         Debug.Assert(projectFilePath.Exists);
         int backupIndex = 0;
         IAbsoluteFilePath backupProjectFilePath;
         // increment backupIndex until we find a backupProjectFilePath that don't exist.
         while (true) {
            var backupProjectFileName = projectFilePath.FileNameWithoutExtension + ".bkp" + backupIndex + projectFilePath.FileExtension;
            backupProjectFilePath = projectFilePath.ParentDirectoryPath.GetChildFileWithName(backupProjectFileName);
            if (!backupProjectFilePath.Exists) {
               break;
            }
            backupIndex++;
         }

         try {
            File.Copy(projectFilePath.ToString(), backupProjectFilePath.ToString());
         }
         catch (Exception ex) {
            throw new Exception("Can't create the backup project file {" + backupProjectFilePath.ToString() + "}.", ex);
         }
         Console.WriteLine("Backup file created: {" + backupProjectFilePath.ToString() + "}.");
      }
   }
}
