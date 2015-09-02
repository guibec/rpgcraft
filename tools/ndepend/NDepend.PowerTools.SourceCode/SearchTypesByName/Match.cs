using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;


namespace NDepend.PowerTools.SearchTypesByName {
   class Match {
      internal Match(int index, int length) {
         Debug.Assert(index >= 0);
         Debug.Assert(length > 0);
         m_Index = index;
         m_Length = length;
      }

      private readonly int m_Index;
      internal int Index { get { return m_Index; } }

      private readonly int m_Length;
      internal int Length { get { return m_Length; } }



      internal static List<Match> ExtractMatchs(string str, string[] matchersLowerCase) {
         Debug.Assert(str != null);
         Debug.Assert(matchersLowerCase != null);
         var matchs = new List<Match>();
         var strLower = str.ToLower();

         // For each matcher extract match(s) in strLower
         foreach (var matcher in matchersLowerCase) {
            var index = 0;
            while (true) {
               index = strLower.IndexOf(matcher, index);
               if (index == -1) { break; }
               matchs.Add(new Match(index, matcher.Length));
               index += matchersLowerCase.Length;
               if (index >= strLower.Length) { break; }
            }
         }

         // At least one match per matcher since types have already been filtered!
         Debug.Assert(matchs.Count >= matchersLowerCase.Length);
         matchs = matchs.OrderBy(match => match.Index).ToList();

         // Remove overlapping match
         var matchsToRemove = new HashSet<Match>();
         for (var i = 0; i < matchs.Count - 1; i++) {
            var matchI = matchs[i];
            var noMatchBeforeIndex = matchI.Index + matchI.Length;
            for (var j = i + 1; j < matchs.Count; j++) {
               var matchJ = matchs[j];
               Debug.Assert(matchJ.Index >= matchI.Index); // Coz sorted
               if (matchJ.Index < noMatchBeforeIndex) {
                  matchsToRemove.Add(matchJ);
               }
            }
         }

         // This algo is not clever enougth to distinguish when AB and BC overlap in ABC to get a ABC match!

         matchs.RemoveAll(matchsToRemove.Contains);
         return matchs;
      }
   }
}
