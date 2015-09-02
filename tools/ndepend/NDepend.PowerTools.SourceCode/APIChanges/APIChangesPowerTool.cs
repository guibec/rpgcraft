
using System.Diagnostics;
using NDepend.CodeModel;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.APIChanges {

   class APIChangesPowerTool : IPowerTool {
      public string Name {
         get { return "Public API Changes (including API Breaking Changes)"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "Show API changes and especially highlights API breaking changes.",
                  "Each kind of API change is matched by a CQLinq rule or query",
                  "available in any default NDepend project.",
               };
         }
      }

 

      public void Run() {
         ICompareContext compareContext;
         if (!ProjectAnalysisUtils.TryGetAssembliesToCompareAndAnalyzeThem(out compareContext)) { return; }
         Debug.Assert(compareContext != null);

         APIChangesDisplayer.Go(compareContext);
      }

   }

 
}
