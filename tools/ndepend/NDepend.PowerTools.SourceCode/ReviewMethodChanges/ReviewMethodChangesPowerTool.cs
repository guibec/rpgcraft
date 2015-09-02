
using System;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.ReviewMethodChanges {
   class ReviewMethodChangesPowerTool : IPowerTool {

      public string Name {
         get { return "Code Review Methods Changed"; }
      }

      public string[] Description {
         get {
            return new[] {
                  " - Up/Down to browse methods changed",
                  " - Enter to compare older and newer versions of a method changed",
                  "",
                  "The tool that compare older and newer source versions of the source file",
                  "can be defined in:",
                  "   NDepend UI > Tools > Options > Build Comparison > Source Files Compare Tool",
                  "",
                  "Note that the panel Search by Changes in NDepend UI", 
                  "proposes more options to code review diff."
               };
         }
      }


      public void Run() {
         ICompareContext compareContext;
         if (!ProjectAnalysisUtils.TryGetAssembliesToCompareAndAnalyzeThem(out compareContext)) { return; }
         Debug.Assert(compareContext != null);

         var methodsChanged = compareContext.NewerCodeBase.Methods.Where(compareContext.CodeWasChanged).Cast<IMember>().ToArray();
         
         Console.WriteLine();
         Console.ForegroundColor = ConsoleColor.Green;
         Console.WriteLine(methodsChanged.Length + " method" + (methodsChanged.Length > 1 ? "s" : "") + " where code has been changed");
         Console.WriteLine();

         var methodsChangedDisplayer = new MethodsChangedDisplayer(methodsChanged, compareContext);
         methodsChangedDisplayer.Display();
      }

   }
}
