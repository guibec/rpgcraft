using System.Diagnostics;
using System.Linq;
using NDepend.Analysis;
using NDepend.CodeQuery;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.TestDebuggerDisplay {
   internal static class TesterDebuggerDisplay {

      //
      // A bit of code to test if DebuggerDisplay works well
      //
      internal static void Go() {

         // DebuggerDisplay of IAnalysisResult
         IAnalysisResult analysisResult;
         if (!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);


         // DebuggerDisplay of IAnalysisResultRef
         var analysisResultRef = analysisResult.AnalysisResultRef;

         // DebuggerDisplay of IProject
         var project = analysisResultRef.Project;

         // DebuggerDisplay of IQuery
         var query = project.CodeQueries.CodeQueriesSet.AllQueriesRecursive.First();

         // DebuggerDisplay of ICodeBase
         var codeBase = analysisResult.CodeBase;

         // DebuggerDisplay of IAssembly
         var assembly = codeBase.Assemblies.First();

         // DebuggerDisplay of INamespace
         var @namespace = codeBase.Namespaces.First();

         // DebuggerDisplay of IType
         var type = codeBase.Types.First();

         // DebuggerDisplay of IField
         var field = codeBase.Fields.First();

         // DebuggerDisplay of IMethod
         var method = codeBase.Methods.Where(m => m.SourceFileDeclAvailable).First();

         // DebuggerDisplay of ISourceFileLine
         var sourceFileLine = method.SourceDecls.First();

         // DebuggerDisplay of ISourceFile
         var sourceFile = sourceFileLine.SourceFile;

         // DebuggerDisplay of IQueryCompiledError
         var queryCompiledError = "query not compilable".Compile(codeBase).QueryCompiledError;

         // DebuggerDisplay of IQueryCompilationError
         var queryCompilationError = queryCompiledError.Errors.First();
      }
   }
}
