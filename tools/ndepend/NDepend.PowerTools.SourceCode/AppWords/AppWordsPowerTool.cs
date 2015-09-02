using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.Analysis;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.AppWords {


   class AppWordsPowerTool : IPowerTool {

      public string Name {
         get { return "AppWords"; }
      }

      public string[] Description {
         get {
            return new string[] {
            "List terms used in assemblies, namespaces, types, methods, fields identifiers",
            "sorted by relevancy, excepting common infrastructure terms.",
            "AppWords is useful to list domain words of an application.",
            "It can also be useful to detect forbidden words and spelling errors.",
         };
         }
      }

      public void Run() {
         IAnalysisResult analysisResult;
         if(!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);

         var codeBase = analysisResult.CodeBase;

         //
         // Get weight for each identifier
         //
         var identifierWeights = codeBase.Assemblies.Select(IdentifierWeightComputer.GetWeigth).ToList();
         identifierWeights.AddRange(codeBase.Namespaces.Select(IdentifierWeightComputer.GetWeigth).ToList());
         identifierWeights.AddRange(codeBase.Types.Select(IdentifierWeightComputer.GetWeigth).ToList());
         identifierWeights.AddRange(codeBase.Methods.Select(IdentifierWeightComputer.GetWeigth).ToList());
         identifierWeights.AddRange(codeBase.Fields.Select(IdentifierWeightComputer.GetWeigth).ToList());


         //
         // Extract words and their weight from identifier
         //
         var dico = new Dictionary<string, float>();
         foreach (var identifierWeight in identifierWeights) {
            if (identifierWeight == null) { continue; }
            var weightNullable = identifierWeight.Weight;
            if(weightNullable == null || weightNullable == 0) { continue; }
            var weight = weightNullable.Value;
            var words = WordExtracter.ExtractWordsFromIdentifier(identifierWeight.Identifier);
            foreach (var word in words) {
               if (!dico.ContainsKey(word)) {
                  dico.Add(word, weight);
                  continue;
               }
               dico[word] += weight;
            }
         }

         //
         // Sort weigh descendant
         //
         var wordWeights = dico.Select(pair => new IdentifierWeight(pair.Key, pair.Value)).ToList();
         wordWeights.Sort((w1, w2) => w1.Weight < w2.Weight ? 1 : w1.Weight > w2.Weight ? -1 : 0);



         //
         // show result
         //
         var listOfLines = (from wordWeight in wordWeights
                            let identifier = wordWeight.Identifier
                            select identifier + (identifier.Length < 30 ? new string(' ', 30 - identifier.Length) : "   ") + "Weight: " + wordWeight.Weight).ToList();

         ShowConsoleListOfLines(listOfLines);
      }



      internal static void ShowConsoleListOfLines(List<string> listOfLines) {
         Debug.Assert(listOfLines != null);
         var nbPage = 0;
         var consoleHeight = Console.WindowHeight - 4;
         bool showAll = false;
         while (true) {
            // Show a page of line!
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine(nbPage * consoleHeight + " - " + (nbPage + 1) * consoleHeight + "     on " + listOfLines.Count);
            Console.ForegroundColor = ConsoleColor.White;
            for (var i = 0; i < consoleHeight; i++) {
               var index = i + nbPage * consoleHeight;
               if (index >= listOfLines.Count) { return; }
               var line = listOfLines[index];
               Console.WriteLine(line);
            }
            nbPage++;

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
