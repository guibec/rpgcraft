using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.PowerTools.SharedUtils;


namespace NDepend.PowerTools.SearchTypesByName {
   class ResultDisplayer {

      internal ResultDisplayer() {
         m_CursorTop = Console.CursorTop +2;
      }

      private readonly int m_CursorTop;
      private const ConsoleColor BACK_COLOR_REGULAR = ConsoleColor.Black;
      private const ConsoleColor FORE_COLOR_REGULAR = ConsoleColor.White;
      private const ConsoleColor FORE_COLOR_ASSEMBLY = ConsoleColor.DarkGray;
      private const ConsoleColor FORE_COLOR_NAMESPACE = ConsoleColor.DarkMagenta;
      private const ConsoleColor FORE_COLOR_TYPE = ConsoleColor.White;
      private const ConsoleColor BACK_COLOR_MATCH = ConsoleColor.DarkCyan;
      private const ConsoleColor BACK_COLOR_SELECTION = ConsoleColor.DarkGreen;
      private const int MAX_TYPES_SHOWN = 15;

      internal event Action<IType[]> OnTypesDisplayedChanged;

      // Mutable fields
      private int m_MaxLinesShown = 0;
      private IType[] m_TypesDisplayed = new IType[0];
      private int[] m_TypesCursorTop = new int[0];
      private string[] m_MatchersLowerCase;

      internal void Display(IEnumerable<IType> typesMatched, string[] matchersLowerCase) {
         Debug.Assert(typesMatched != null);
         Debug.Assert(matchersLowerCase != null);

         // Init this display 
         var nbTypes = typesMatched.Count();
         var nbTypesToDisplay = nbTypes > MAX_TYPES_SHOWN ? MAX_TYPES_SHOWN : nbTypes;
         m_TypesDisplayed = typesMatched.Take(nbTypesToDisplay).ToArray();
         m_TypesCursorTop = new int[nbTypesToDisplay];
         m_MatchersLowerCase = matchersLowerCase;

         // Erase previous results.
         Console.CursorTop = m_CursorTop;
         Console.CursorLeft = 0;
         ConsoleUtils.ShowNLinesOnConsole(m_MaxLinesShown, BACK_COLOR_REGULAR);
         Console.CursorTop = m_CursorTop;
         Console.CursorLeft = 0;

         // Show # types matched
         Console.BackgroundColor = BACK_COLOR_REGULAR;
         Console.ForegroundColor = FORE_COLOR_REGULAR;

         Console.WriteLine(nbTypes + " type" + (nbTypes > 1 ? "s" : "") + " matched. " + (nbTypes > MAX_TYPES_SHOWN ? "First " + nbTypesToDisplay + " are." : ""));
         Console.WriteLine();

         // Show all types names
         for(var i=0; i< nbTypesToDisplay; i++) {
            m_TypesCursorTop[i] = Console.CursorTop;
            DisplayTypeName(m_TypesDisplayed[i], matchersLowerCase);
         }

         // Update m_MaxLinesShown 
         var nbLines = Console.CursorTop - m_CursorTop;
         if (nbLines > m_MaxLinesShown) { m_MaxLinesShown = nbLines; }

         if (OnTypesDisplayedChanged != null) {
            OnTypesDisplayedChanged(m_TypesDisplayed);
         }
      }


      internal enum Select {
         Yes,
         No
      }


      internal void DisplayTypeName(IType type, Select select) {
         Debug.Assert(type != null);
         int index = new List<IType>(m_TypesDisplayed).IndexOf(type);
         Debug.Assert(index >= 0);
         Debug.Assert(index <= m_TypesDisplayed.Length);
         Debug.Assert(m_TypesDisplayed.Length > 0);
         Console.BackgroundColor = select == Select.Yes ? BACK_COLOR_SELECTION : BACK_COLOR_REGULAR;
         Console.CursorLeft = 0;
         Console.CursorTop = m_TypesCursorTop[index];
         DisplayTypeName(type, m_MatchersLowerCase);
      }


      private static void DisplayTypeName(IType type, string[] matchersLowerCase) {
         Debug.Assert(type != null);
         Debug.Assert(matchersLowerCase != null);
         Console.ForegroundColor = FORE_COLOR_ASSEMBLY;
         Console.Write(type.ParentAssembly.Name + "::");
         Console.ForegroundColor = FORE_COLOR_NAMESPACE;
         Console.Write(type.ParentNamespace.Name + ".");


         // Extract matchs!
         var matchs = Match.ExtractMatchs(type.Name, matchersLowerCase);
         Debug.Assert(matchs.Count > 0);

         // Show before first match eventually
         Console.ForegroundColor = FORE_COLOR_TYPE;
         if (matchs[0].Index > 0) {
            Console.Write(type.Name.Substring(0, matchs[0].Index));
         }

         var currentBackColor = Console.BackgroundColor;
         for (var i = 0; i < matchs.Count; i++) {
            // Write a match
            var match = matchs[i];
            var matchString = type.Name.Substring(match.Index, match.Length);
            Console.BackgroundColor = BACK_COLOR_MATCH;
            Console.Write(matchString);

            // Write non match until next match or end
            var nextIndex = i == matchs.Count - 1 ? type.Name.Length : matchs[i + 1].Index;
            Console.BackgroundColor = currentBackColor;
            var endMatchIndex = match.Index + match.Length;
            Debug.Assert(nextIndex >= endMatchIndex);
            var betweenMatchString = type.Name.Substring(endMatchIndex, nextIndex - endMatchIndex);
            Console.Write(betweenMatchString);
         }
         Console.WriteLine();
      }

   }
}
