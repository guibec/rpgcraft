using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.PowerTools.SharedUtils;


namespace NDepend.PowerTools.APIChanges {

   internal class ChangeRecord {
      internal ChangeRecord(string descrition, IEnumerable<IMember> members, ConsoleColor consoleColor) {
         Debug.Assert(descrition != null);
         Debug.Assert(members != null);
         m_Description = descrition;
         m_Members = members;
         m_ConsoleColor = consoleColor;
      }
      internal readonly string m_Description;
      internal readonly IEnumerable<IMember> m_Members;
      internal readonly ConsoleColor m_ConsoleColor;
   }



   internal static class APIChangesDisplayer {

      internal static void Go(ICompareContext compareContext) {
         Debug.Assert(compareContext != null);

         var olderCodeBase = compareContext.OlderCodeBase;
         var newerCodeBase = compareContext.NewerCodeBase;


         //
         // Types not publicly visible anymore
         //
         var publicTypesRemoved =
                               from t in olderCodeBase.Application.Types
                               let tNewer = compareContext.NewerVersion(t) // notice we don't use the syntax t.NewerVersion specific to CQLinq
                               where
                               t.IsPubliclyVisible &&
                               (tNewer == null || !tNewer.IsPubliclyVisible)
                               select t;

         //
         // Methods not publicly visible anymore
         //
         var publicMethodsRemoved =
                               from m in olderCodeBase.Application.Methods
                               let mNewer = compareContext.NewerVersion(m) 
                               where
                                  m.IsPubliclyVisible &&
                                  !m.ParentType.IsDelegate && // Don't show new delegate generated methods!
                                  (mNewer == null || !mNewer.IsPubliclyVisible) &&
                                  !compareContext.WasRemoved(m.ParentType)  && // (not including methods in public types removed)
                                  // There is no base methods that is present in both builds
                                  m.OverriddensBase.Where(compareContext.IsPresentInBothBuilds).Count() == 0
                               select m;

         //
         // Fields not publicly visible anymore
         //
         var publicFieldsRemoved =
                               from f in olderCodeBase.Application.Fields
                               let tNewer = compareContext.NewerVersion(f)
                               where
                               f.IsPubliclyVisible &&
                               (tNewer == null || !tNewer.IsPubliclyVisible) &&
                               !compareContext.WasRemoved(f.ParentType)  // (not including fields in public types removed)
                               select f;


         //
         // Public Base classes or interfaces with new abstract methods
         //
         var newAbstractMethodsOnExistingTypes =
                               from m in newerCodeBase.Application.Methods
                               let mOlder = compareContext.OlderVersion(m)
                               where
                                 m.IsAbstract &&
                                 m.IsPubliclyVisible && !m.IsThirdParty &&
                                 compareContext.IsPresentInBothBuilds(m.ParentType) &&
                                 (mOlder == null || !mOlder.IsPubliclyVisible)
                               select m; ;


         //
         // New public types
         //
         var newPublicTypes = from t in newerCodeBase.Application.Types
                              let tOlder = compareContext.OlderVersion(t) 
                              where
                              t.IsPubliclyVisible &&
                              (tOlder == null || !tOlder.IsPubliclyVisible)
                              select t;


         //
         // New public methods
         //
         var newPublicMethods = from m in newerCodeBase.Application.Methods
                                let mOlder = compareContext.OlderVersion(m) 
                                where
                                m.IsPubliclyVisible && !m.IsThirdParty &&
                                !m.ParentType.IsDelegate && // Don't show new delegate generated methods!
                                (mOlder == null || !mOlder.IsPubliclyVisible) &&
                                !compareContext.WasAdded(m.ParentType) // (not including methods in new public types)
                                select m;


         //
         // New public fields
         //
         var newPublicFields = from f in newerCodeBase.Application.Fields 
                               let fOlder = compareContext.OlderVersion(f) 
                               where
                               f.IsPubliclyVisible &&
                               (fOlder == null || !fOlder.IsPubliclyVisible) &&
                               !compareContext.WasAdded(f.ParentType)  // (not including fields in new public types)
                               select f;


         var changes = new List<ChangeRecord> {
               // API breaking changes
               new ChangeRecord(" public types removed", publicTypesRemoved.Cast<IMember>(), COLOR_WARNING),
               new ChangeRecord(" public methods removed (not including methods in public types removed)", publicMethodsRemoved.Cast<IMember>(), COLOR_WARNING),
               new ChangeRecord(" public fields removed (not including fields in public types removed)", publicFieldsRemoved.Cast<IMember>(), COLOR_WARNING),
               new ChangeRecord(" public asbtract methods added on existing type", newAbstractMethodsOnExistingTypes.Cast<IMember>(), COLOR_WARNING),

               // API changes
               new ChangeRecord(" new public types", newPublicTypes.Cast<IMember>(), COLOR_DEFAULT),
               new ChangeRecord(" new public methods (not including methods in new public types)", newPublicMethods.Cast<IMember>(), COLOR_DEFAULT),
               new ChangeRecord(" new public fields (not including fields in new public types)", newPublicFields.Cast<IMember>(), COLOR_DEFAULT)
                                              };
         changes.RemoveAll(changeRecord => changeRecord.m_Members.Count() == 0);

         ShowChanges(changes);
      }

      private const ConsoleColor COLOR_WARNING = ConsoleColor.Yellow;
      private const ConsoleColor COLOR_NO_WARNING = ConsoleColor.Green;
      private const ConsoleColor COLOR_DEFAULT = ConsoleColor.White;

      private static void ShowChanges(List<ChangeRecord> changes) {
         Debug.Assert(changes != null);

         if (changes.Count == 0) {
            Console.ForegroundColor = COLOR_NO_WARNING;
            Console.WriteLine("No API changes");
            return;
         }


         //
         // Show changes
         //
         while (true) {

            ConsoleUtils.ShowNLinesOnConsole(3, ConsoleColor.Black);

            if (changes.Where(changeTmp => changeTmp.m_ConsoleColor == COLOR_WARNING).Count() == 0) {
               Console.ForegroundColor = COLOR_NO_WARNING;
               Console.WriteLine("No API breaking change detected!");
            } else {
               Console.ForegroundColor = COLOR_WARNING;
               Console.WriteLine("API breaking change detected! (in Yellow)");
            }

            //
            // Show changes menu
            //
            var aCharNumber = Convert.ToInt16('a');
            for (var i = 0; i <= changes.Count; i++) {
               var changeDescription = "Exit";
               if (i != changes.Count) {
                  var changeTmp = changes[i];
                  Console.ForegroundColor = changeTmp.m_ConsoleColor;
                  changeDescription = changeTmp.m_Members.Count() + changeTmp.m_Description;
               }
               Console.WriteLine(" " + Convert.ToChar(aCharNumber + i) + ")  " + changeDescription);
            }
            Console.ForegroundColor = COLOR_DEFAULT;


            //
            // Get user choice
            //
            int changeIndex;
            while (true) {
               Console.Write("Enter change key:");
               var keyChar = Char.ToLower(Console.ReadKey().KeyChar);
               Console.WriteLine("");
               changeIndex = Convert.ToInt16(keyChar) - aCharNumber;
               if (changeIndex < 0 || changeIndex > changes.Count) {
                  Console.Write(keyChar + " is not a valid key!");
                  continue;
               }
               break;
            }

            if (changeIndex == changes.Count) { break; }
            var change = changes[changeIndex];


            //
            // Show changes
            //
            var membersDisplayer = new MembersDisplayer(change.m_Members.ToList());
            membersDisplayer.Display();

         }
      }
   }
}
