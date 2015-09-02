using System;
using System.Diagnostics;

namespace NDepend.PowerTools.CodeQueryConsole {
   // Cell content is a list of StringColored
   class StringColored {
      internal StringColored(string @string, ConsoleColor color) {
         Debug.Assert(@string != null);
         m_String = @string;
         m_Color = color;
      }
      private readonly string m_String;
      internal string String { get { return m_String; } }
      private readonly ConsoleColor m_Color;
      internal ConsoleColor Color { get { return m_Color; } }
   }
}
