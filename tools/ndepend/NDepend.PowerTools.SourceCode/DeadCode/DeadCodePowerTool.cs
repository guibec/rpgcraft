using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.Analysis;
using NDepend.CodeModel;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;


namespace NDepend.PowerTools.DeadCode {

   internal class DeadCodeRecord {
      internal DeadCodeRecord(string descrition, IEnumerable<IMember> members) {
         Debug.Assert(descrition != null);
         Debug.Assert(members != null);
         m_Description = descrition;
         m_Members = members;
      }
      internal readonly string m_Description;
      internal readonly IEnumerable<IMember> m_Members;
   }


   class DeadCodePowerTool : IPowerTool {
      public string Name {
         get { return "Find potential Dead Code"; }
      }

      public string[] Description {
         get {
            return new[] {
                  "Algorithms used to find dead types, methods and fields ...",
                  "... are the same as the default CQLinq dead code rules algorithms.",
               };
         }
      }


      private const ConsoleColor COLOR_WARNING = ConsoleColor.Yellow;
      private const ConsoleColor COLOR_NO_WARNING = ConsoleColor.Green;
      private const ConsoleColor COLOR_DEFAULT = ConsoleColor.White;

      public void Run() {
         IAnalysisResult analysisResult;
         if (!ProjectAnalysisUtils.TryChooseAnalysisResult(out analysisResult)) { return; }
         Debug.Assert(analysisResult != null);

         var codeBase = analysisResult.CodeBase;

         // make usage of the default NDepend attribute: NDepend.Attributes.IsNotDeadCodeAttribute
         var isNotDeadCodeAttribute = codeBase.Types.WithFullName("NDepend.Attributes.IsNotDeadCodeAttribute").SingleOrDefault();
         var funcHasAttribute = new Func<IMember,bool>(member =>
             (isNotDeadCodeAttribute != null && member.HasAttribute(isNotDeadCodeAttribute))  
          );


         //
         // Apply dead code algo!
         //
         var deadTypesMetric = DeadCodeAlgo.FindDeadTypes(codeBase, funcHasAttribute);
         var deadMethodsMetric = DeadCodeAlgo.FindDeadMethods(codeBase, funcHasAttribute);
         var deadFields = DeadCodeAlgo.FindDeadFields(codeBase, funcHasAttribute);

         var deadCodeRecords = new List<DeadCodeRecord>();
         deadCodeRecords.Add(new DeadCodeRecord(" unused types", deadTypesMetric.Where(v => v.Value == 0).Select(v => v.CodeElement).Cast<IMember>().ToArray()));
         deadCodeRecords.Add(new DeadCodeRecord(" types used only by unused types", deadTypesMetric.Where(v => v.Value > 0).Select(v => v.CodeElement).Cast<IMember>().ToArray()));

         deadCodeRecords.Add(new DeadCodeRecord(" unused methods", deadMethodsMetric.Where(v => v.Value == 0).Select(v => v.CodeElement).Cast<IMember>().ToArray()));
         deadCodeRecords.Add(new DeadCodeRecord(" methods used only by unused methods", deadMethodsMetric.Where(v => v.Value > 0).Select(v => v.CodeElement).Cast<IMember>().ToArray()));

         deadCodeRecords.Add(new DeadCodeRecord(" unused fields", deadFields.Cast<IMember>().ToArray()));

         //
         // No dead code found!
         //
         var deadCodeFound = deadCodeRecords.Where(record => record.m_Members.Count() > 0).Count() > 0;
         if (!deadCodeFound) {
            Console.ForegroundColor = COLOR_NO_WARNING;
            Console.WriteLine("No dead code found!");
            return;
         }


         //
         // Show dead code
         //
         while (true) {

            ConsoleUtils.ShowNLinesOnConsole(3, ConsoleColor.Black);

            if (deadCodeRecords.Where(record => record.m_Members.Count() > 0).Count() == 0) {
               Console.ForegroundColor = COLOR_NO_WARNING;
               Console.WriteLine("No dead code found!");
            } else {
               Console.ForegroundColor = COLOR_WARNING;
               Console.WriteLine("Dead code found! (in Yellow)");
            }

            //
            // Show dead code menu
            //
            var aCharNumber = Convert.ToInt16('a');
            for (var i = 0; i <= deadCodeRecords.Count; i++) {
               var description = "Exit";
               if (i != deadCodeRecords.Count) {
                  var record = deadCodeRecords[i];
                  Console.ForegroundColor = record.m_Members.Count() == 0 ? COLOR_NO_WARNING : COLOR_WARNING;
                  description = record.m_Members.Count() + record.m_Description;
               }
               Console.WriteLine(" " + Convert.ToChar(aCharNumber + i) + ")  " + description);
            }
            Console.ForegroundColor = COLOR_DEFAULT;


            //
            // Get user choice
            //
            int deadCodeIndex;
            while (true) {
               Console.Write("Enter key:");
               var keyChar = Char.ToLower(Console.ReadKey().KeyChar);
               Console.WriteLine("");
               deadCodeIndex = Convert.ToInt16(keyChar) - aCharNumber;
               if (deadCodeIndex < 0 || deadCodeIndex > deadCodeRecords.Count) {
                  Console.Write(keyChar + " is not a valid key!");
                  continue;
               }
               break;
            }

            if (deadCodeIndex == deadCodeRecords.Count) { break; }
            var recordTmp = deadCodeRecords[deadCodeIndex];


            //
            // Show dead members
            //
            var membersDisplayer = new MembersDisplayer(recordTmp.m_Members.ToList());
            membersDisplayer.Display();
         }
      }


   }
}
