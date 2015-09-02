using System;
using System.Diagnostics;
using System.Linq;
using NDepend.Analysis;
using NDepend.CodeModel;
using NDepend.CodeQuery;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.CodeQueryConsole {


   // Some queries used for smoke test:
   // from t in Types where t.IsStatic select new { t, t.NbLinesOfCode, t.IsAbstract, t.NbMethods }
   // from n in Namespaces select new { n, n.ParentAssembly, n.NamespacesUsed }
   class CodeQueryConsolePowerTool : IPowerTool {
      public string Name {
         get { return "Query Code with CQLinq"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "Edit CQLinq and CQL code query and show results.",
                  "It is recommended to edit CQLinq queries and see query result",
                  "in NDepend UI thanks to intellisense and result display facilities.",
                  "But we notice many programmers enjoy console edition mode!",
                  " ->  F5 key to run the edited query.",
                 @" ->  PageUp/PageDow shows previous/next edited queries",
                  " ->  Previous/next edited queries list is initially filled",
                  "     with queries of the NDepend project.",
                  " ->  Ctrl+C - Ctrl+V - Ctrl+X  Copy/Paste/Cut from clipboard supported.",
                  " ->  Ctrl+A select all and Tab+Arrow selection supported.",
                  " ->  Ctrl+Z  Ctrl+Y  Undo/Redo supported.",
                  " ->  ESC key to exit the power tool.",
               };
         }
      }


      public void Run() {
         IAnalysisResult analysisResult;
         if (!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);
         
         var codeBase = analysisResult.CodeBase;
         Func<string, IQueryCompiled> compileQueryProc = queryString => queryString.Compile( codeBase);

         // ... but if we can get a compareContext, then compile and execute the query against the compareContext
         ICompareContext compareContext;
         if (ProjectAnalysisUtils.TryGetCompareContextDefinedByBaseline(analysisResult, out compareContext)) {
            Debug.Assert(compareContext != null);
            compileQueryProc = queryString => queryString.Compile(compareContext);
         }

         //
         // Fill queriesPreviouslyEdited with current project queries
         //
         IPreviousNextStack<string> queriesPreviouslyEdited = new PreviousNextStack<string>(
            analysisResult.AnalysisResultRef.Project.CodeQueries.CodeQueriesSet.AllQueriesRecursive.Reverse().Select(q => q.QueryString),
            "",
            (s1,s2) => s1 == s2);

         // With this line, make sure to begin with the first queries of the project.
         queriesPreviouslyEdited.GetPrevious();


         //
         // Loop for each query edition
         //
         while (true) {
            IQueryCompiled queryCompiled;
            using (var queryEditSession = new QueryEditSession(queriesPreviouslyEdited)) {
               var queryString = queryEditSession.GetQueryString();
COMPILE_QUERY:
               Console.BackgroundColor = ConsoleColor.Black;
               Console.ForegroundColor = ConsoleColor.White;
               if (queryString == null) { break; }

               // Try compile query
               queryCompiled = compileQueryProc(queryString);
               var queryCompiledError = queryCompiled.QueryCompiledError;
               if (queryCompiledError != null) {
                  queryString = queryEditSession.ShowCompilatioErrorsAndThenGetQueryString(queryCompiledError);
                  goto COMPILE_QUERY;
               }
            }

            // Execute query compiled
            var queryCompiledSuccess = queryCompiled.QueryCompiledSuccess;
            Debug.Assert(queryCompiledSuccess != null);
            var result = queryCompiledSuccess.Execute();
            if (result.Status != QueryExecutionStatus.Success) {
               var exception = result.Exception;
               // The error must be an Exception thrown by the query, since we don't use the Execute(...) overload with time-out!
               Debug.Assert(exception != null);
               DisplayQueryThrowAnException(exception);
               continue;
            }

            QueryExecutionResultDisplayer.Go(result.SuccessResult);
            Console.WriteLine();
         }

      }

      private static void DisplayQueryThrowAnException(Exception exception) {
         Debug.Assert(exception != null);
         Console.BackgroundColor = ConsoleColor.Black;
         Console.ForegroundColor = ConsoleColor.Red;
         Console.WriteLine("Exception of type {" + exception.GetType().Name + "} thrown by the query.");
         Console.WriteLine("Exception.Message: " + exception.Message);
         Console.WriteLine("Exception.StackTrace: " + exception.StackTrace);
         Console.BackgroundColor = ConsoleColor.Black;
         Console.ForegroundColor = ConsoleColor.White;
      }
   }
}
