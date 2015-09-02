
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.Analysis;
using NDepend.CodeModel;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.SearchTypesByName {
   class SearchTypesByNamePowerTool : IPowerTool {

      public string Name {
         get { return "Search Types by Name"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "Search types by name matching.",
                  " - Up/Down to browse types",
                  " - Enter to open type source file decl",
                  " - The search is case insensitive",
                  " - Third-party types are not taken account",
                  " - Several matchers strings can be separated by spaces",
                  " - ESC to quit",
                  "Note that the Search panel in NDepend UI proposes many more options."
               };
         }
      }


      public void Run() {
         IAnalysisResult analysisResult;
         if (!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);

         var codeBase = analysisResult.CodeBase;
         var types = codeBase.Application.Types;

         var matchEditor = new MatchEditor();
         var resultDisplayer = new ResultDisplayer();
         var resultSelector = new ResultSelector(resultDisplayer);
         matchEditor.Display();

         while (true) {
            ConsoleKeyInfo consoleKeyInfo;
            bool matchersChanged;
            matchEditor.ReadKey(out consoleKeyInfo, out matchersChanged);

            if (!matchersChanged) {
               switch(consoleKeyInfo.Key) {
                  case ConsoleKey.Escape: 
                     return;

                  case ConsoleKey.UpArrow:
                     resultSelector.UpArrow();
                     continue;
                  case ConsoleKey.DownArrow:
                     resultSelector.DownArrow();
                     continue;
                  case ConsoleKey.Enter:
                     resultSelector.Enter();
                     continue;
               }
               continue;
            }

            var matchersLowerCase = matchEditor.GetMatchersLowerCase();
            var typesMatched = SearchTypes(types, matchersLowerCase);
            resultDisplayer.Display(typesMatched, matchersLowerCase);
         }

      }

      private static IEnumerable<IType> SearchTypes(IEnumerable<IType> types, string[] matchersLowerCase) {
         Debug.Assert(types != null);
         Debug.Assert(matchersLowerCase != null);
         if (matchersLowerCase.Length == 0) { return new IType[0]; }
         return (from type in types
                 let typeNameLower = type.Name.ToLower()
                 where matchersLowerCase.All(typeNameLower.Contains)
                 select type).ToArray();
      }
   }
}
