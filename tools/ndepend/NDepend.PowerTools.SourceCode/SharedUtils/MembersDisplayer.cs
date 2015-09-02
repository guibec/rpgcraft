using System;
using System.Collections.Generic;
using System.Diagnostics;
using NDepend.CodeModel;

namespace NDepend.PowerTools.SharedUtils {
   public class MembersDisplayer : CodeElementsDisplayerBase<IMember> {

      internal MembersDisplayer(IList<IMember> members) {
         Debug.Assert(members != null);
         m_Members = members;
      }

      private readonly IList<IMember> m_Members;

      private int m_Index;
      protected override IMember GetNext() {
         if (m_Index == m_Members.Count) { return null; }
         var record = m_Members[m_Index];
         m_Index++;
         return record;
      }

      protected override void Display(IMember member) {
         Debug.Assert(member != null);
         ConsoleUtils.ShowConsoleMemberFullNamePrefixedWithAssembly(member);
         Console.WriteLine();  // One line separator!
      }

      protected override ICodeElement GetCodeElement(IMember member) {
         return member;
      }


   }
}
