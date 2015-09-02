using System;


namespace NDepend.PowerTools.CodeQueryConsole {

   partial class QueryEditSession {

      partial class ConsoleWriter {

         class Highlight {
            internal Highlight(int startPosition, int length, ConsoleColor foregroundColor, ConsoleColor backgroundColor) {
               m_StartPosition = startPosition;
               m_Length = length;
               m_ForegroundColor = foregroundColor;
               m_BackgroundColor = backgroundColor;
            }

            private readonly int m_StartPosition;
            internal int StartPosition { get { return m_StartPosition; } }

            private readonly int m_Length;
            internal int Length { get { return m_Length; } }

            private readonly ConsoleColor m_BackgroundColor;
            internal ConsoleColor BackgroundColor { get { return m_BackgroundColor; } }

            private readonly ConsoleColor m_ForegroundColor;
            internal ConsoleColor ForegroundColor { get { return m_ForegroundColor; } }
         } 

      }
   }
}
