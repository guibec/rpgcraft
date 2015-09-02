using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using NDepend.Analysis;
using NDepend.CodeModel;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.SearchForDuplicateCode {
   class SearchForDuplicateCodePowerTool : IPowerTool {
       public string Name {
         get { return "Search for Duplicate Code"; }
      }

      public string[] Description {
         get {
            return new [] {
                  "Use an original heuristic to spot potential duplicate code.",
                  "Two methods that are calling a same set",
                  "of N methods (or fields) is considered suspect.",
                  "The more methods are calling this set, the more it is considered suspect."
               };
         }
      }

      // N represents the number of same members, 2 or more methods are calling to be considered as containing duplicate code.
      // 6 seems to be a good value from our tests but you can change it and try yourself.
      public const int N = 6;  


      public void Run() {
         IAnalysisResult analysisResult;
         if (!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);

         var codeBase = analysisResult.CodeBase;

         Console.CursorVisible = false;
         if (codeBase.NbILInstructions > 500 * 1000) {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("   ***  The search for duplicate can take a few seconds to a few minutes for large application   ***   ");
            Console.ForegroundColor = ConsoleColor.White;
         }
         var stopwatch = new Stopwatch();
         stopwatch.Start();

         //
         // 1) Find pairs of methods that call N or more same methods/fields.
         //
         Console.WriteLine("1) Find pairs of methods that call N or more same methods/fields.");
         var suspectSetsWith2Callers = SuspectSetsWith2CallersComputer.Go(codeBase, N);

         //
         // 2) Merge suspectSets with 2 methods callers methods into some suspectSets with 2 or more methods callers.
         //
         Console.WriteLine("2) Merge suspectSets with 2 methods callers methods into some suspectSets with 2 or more methods callers.");
         var suspectSetsMerged = SuspectSetsMerger.Go(suspectSetsWith2Callers, N);

         //
         // 3) Sort suspectSets, to try showing most suspect ones first.
         //
         Console.WriteLine("3) Sort suspectSets, to try showing most suspect ones first.");
         SuspectSetsSorter.Go(suspectSetsMerged);

         Console.WriteLine("Search for duplicate duration:" + stopwatch.Elapsed.ToString());

         //
         // 4) Show results!
         //
         ShowSuspectSet(suspectSetsMerged);
      }


      private static void ShowSuspectSet(IList<SuspectSet> suspectSets) {
         Debug.Assert(suspectSets != null);
         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine("   ***  " + suspectSets.Count + " suspect sets found!   ***   ");
         Console.CursorVisible = true;

         bool showAll = false;
         foreach (var suspectSet in suspectSets) {
            Debug.Assert(suspectSet != null);
            Console.ForegroundColor = ConsoleColor.White;
            Console.BackgroundColor = ConsoleColor.DarkGray;

            var nbMethodsCalled = suspectSet.MethodsCalled.Count();
            var nbFieldsRead = suspectSet.FieldsRead.Count();
            var nbFieldsAssigned = suspectSet.FieldsAssigned.Count();

            var sb = new StringBuilder("These " + suspectSet.MethodsCaller.Count + " methods");
            if (nbMethodsCalled > 0) {
               sb.Append(" are calling same " + nbMethodsCalled + " method" + (nbMethodsCalled > 1 ? "s" : ""));
            }
            if (nbFieldsRead > 0) {
               sb.Append(", are reading same " + nbFieldsRead + " field" + (nbFieldsRead > 1 ? "s" : ""));
            }
            if (nbFieldsAssigned > 0) {
               sb.Append(", are assigning same " + nbFieldsAssigned + " field" + (nbFieldsAssigned > 1 ? "s" : ""));
            }
            sb.Append(".");
            Console.WriteLine(sb.ToString());

            Console.BackgroundColor = ConsoleColor.Black;
            foreach (var method in suspectSet.MethodsCaller) {
               Console.Write("  -> ");
               ConsoleUtils.ShowConsoleMemberFullName(method);
            }

            if (nbMethodsCalled > 0) {
               Console.BackgroundColor = ConsoleColor.DarkGray;
               Console.WriteLine(nbMethodsCalled + " method" + (nbMethodsCalled > 1 ? "s" : "") + " called are:");
               Console.BackgroundColor = ConsoleColor.Black;
               foreach (var method in suspectSet.MethodsCalled) {
                  Console.Write("  -> ");
                  ConsoleUtils.ShowConsoleMemberFullName(method);
               }
            }
            if (nbFieldsRead > 0) {
               Console.BackgroundColor = ConsoleColor.DarkGray;
               Console.WriteLine(nbFieldsRead + " field" + (nbFieldsRead > 1 ? "s" : "") + " read are:");
               Console.BackgroundColor = ConsoleColor.Black;
               foreach (var field in suspectSet.FieldsRead) {
                  Console.Write("  -> ");
                  ConsoleUtils.ShowConsoleMemberFullName(field);
               }
            }
            if (nbFieldsAssigned > 0) {
               Console.BackgroundColor = ConsoleColor.DarkGray;
               Console.WriteLine(nbFieldsAssigned + " field" + (nbFieldsAssigned > 1 ? "s" : "") + " assigned are:");
               Console.BackgroundColor = ConsoleColor.Black;
               foreach (var field in suspectSet.FieldsAssigned) {
                  Console.Write("  -> ");
                  ConsoleUtils.ShowConsoleMemberFullName(field);
               }
            }

            // Ask all/next/quit?
            ConsoleUtils.ShowNLinesOnConsole(3, ConsoleColor.Black);
            if (showAll) {
               continue;
            }
ASK_AGAIN:
            switch (ConsoleAskShowOpenDeclNextAllStop()) {
               case OpenDeclAllNextQuit.OpenDecl:
                  OpenCallerMethodsDecl(suspectSet.MethodsCaller);
                  goto ASK_AGAIN;
               case OpenDeclAllNextQuit.Next:
                  continue;
               case OpenDeclAllNextQuit.All:
                  showAll = true;
                  continue;
               default:
                  // Quit
                  return;
            }
         }
      }

      private static void OpenCallerMethodsDecl(IList<IMethod> callersMethod) {
         Debug.Assert(callersMethod != null);
         Debug.Assert(callersMethod.Count >= 2);

         //
         // It seems preferable to open the caller methods source file decl in VS than trying to compare 2 of them
         // with a source file compare tool, since they can be in the same file, in that case not easy to compare 2 different methods
         // or if they are declared in 2 distinct source files, these files are likely incomparable coz too different.
         //
         
         // Open methods source file decl with a source file decl available
         foreach (var method in callersMethod) {
            if (!method.SourceFileDeclAvailable) { continue; }
            method.TryOpenSource();
         }


         /* Algo to compare 2 methods decl

         ISourceFileLine sourceFileLineA = null, sourceFileLineB = null;
         // Get the 2 first methods with a source file decl available
         foreach (var method in callersMethod) {
            if (!method.SourceFileDeclAvailable) { continue; }
            if (sourceFileLineA == null) {
               sourceFileLineA = method.SourceDecl;
               continue;
            }
            sourceFileLineB = method.SourceDecl;
            break;
         }

        
         ConsoleUtils.TryCompareSource(sourceFileLineA, sourceFileLineB, "Cannot find 2 callers methods with source file decl available");
          * */
      }



      enum OpenDeclAllNextQuit {
         OpenDecl,
         All,
         Next,
         Quit
      }

      static OpenDeclAllNextQuit ConsoleAskShowOpenDeclNextAllStop() {
         var cursorTop = Console.CursorTop;
         Console.WriteLine();
         Console.CursorLeft = 0;
         Console.BackgroundColor = ConsoleColor.Black;
         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine("Open callers methods decls?  o   Show Next?  n     Show All?  a     Stop? any key");
         Console.ForegroundColor = ConsoleColor.White;

         var keyChar = Char.ToLower(Console.ReadKey().KeyChar);

         if (keyChar == 'o'  || keyChar == 'n' || keyChar == 'a') {
            Console.CursorTop = cursorTop;
            ConsoleUtils.ShowNLinesOnConsole(4, ConsoleColor.Black);
            Console.CursorTop = cursorTop;
            // HACK: need to erase a second time, sometime erasing just once is not enought??
            ConsoleUtils.ShowNLinesOnConsole(4, ConsoleColor.Black);
            Console.CursorTop = cursorTop;
            switch(keyChar) {
               case 'o': return OpenDeclAllNextQuit.OpenDecl;
               case 'n': return OpenDeclAllNextQuit.Next;
               case 'a': return OpenDeclAllNextQuit.All;
            }
         }

         return OpenDeclAllNextQuit.Quit;
      }
      

   }
}
