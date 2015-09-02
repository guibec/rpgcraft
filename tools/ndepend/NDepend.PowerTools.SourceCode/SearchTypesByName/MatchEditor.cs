

using System;
using System.Diagnostics;
using System.Linq;

namespace NDepend.PowerTools.SearchTypesByName {
   class MatchEditor {

      internal MatchEditor() {
         m_CursorTop = Console.CursorTop;
      }

      private readonly int m_CursorTop;

      private const char SPLIT_CHAR = ' ';
      private const string PROMPTER = "> ";
      private const ConsoleColor s_PrompterBackColor = ConsoleColor.DarkGray;
      private const ConsoleColor s_PrompterForeColor = ConsoleColor.White;

      private const ConsoleColor s_EditorBackColor = ConsoleColor.White;
      private const ConsoleColor s_EditorForeColor = ConsoleColor.Black;


      private string m_MatchString = "";
      private int m_CursorLeft = PROMPTER.Length;

      internal void ReadKey(out ConsoleKeyInfo consoleKeyInfo, out bool matchersChanged) {
         var x = m_CursorLeft - PROMPTER.Length;
         Debug.Assert(x <= m_MatchString.Length);

         Console.CursorTop = m_CursorTop;
         Console.CursorLeft = m_CursorLeft;

         consoleKeyInfo = Console.ReadKey();
         matchersChanged = false;

         var keyChar = consoleKeyInfo.KeyChar;

         if (Char.IsLetterOrDigit(keyChar) || keyChar == SPLIT_CHAR) {
            if (m_MatchString.Length == Console.WindowWidth - PROMPTER.Length - 1) { return; }
            m_MatchString = m_MatchString.Insert(x, keyChar.ToString());
            matchersChanged = true;
            m_CursorLeft++;

         } else switch (consoleKeyInfo.Key) {
            case ConsoleKey.RightArrow:
               if (x == m_MatchString.Length) { break; }
               m_CursorLeft++;
               break;
            case ConsoleKey.LeftArrow:
               if (x == 0) { break; }
               m_CursorLeft--;
               break;
            case ConsoleKey.Backspace:
               if (x == 0 || m_MatchString.Length == 0) { break; }
               m_CursorLeft--;
               m_MatchString = 
                  m_MatchString.Substring(0, x-1) + 
                  (x == m_MatchString.Length ? "" : m_MatchString.Substring(x, m_MatchString.Length - x));
               matchersChanged = true;
               break;
         }

         Display();
      }

      // Write prompter + match string
      internal void Display() {
         Console.BackgroundColor = s_PrompterBackColor;
         Console.ForegroundColor = s_PrompterForeColor;
         Console.CursorTop = m_CursorTop;
         Console.CursorLeft = 0;
         Console.Write(PROMPTER);
         Console.BackgroundColor = s_EditorBackColor;
         Console.ForegroundColor = s_EditorForeColor;
         Console.Write(m_MatchString);
         Console.Write(new string(' ', Console.WindowWidth - PROMPTER.Length - m_MatchString.Length));
      }


      internal string[] GetMatchersLowerCase() {
         var matchers =  m_MatchString.Split(new [] { SPLIT_CHAR }).ToList();
         return matchers
                  .Where(matcher => matcher != "")
                  .Select(matcher => matcher.ToLower())
                  .ToArray();
      }
   }
}
