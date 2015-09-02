


using System;
using System.Diagnostics;

namespace NDepend.PowerTools.CodeQueryConsole {

   partial class QueryEditSession {
      class KeyHandlerChar : IKeyHandler {


         internal KeyHandlerChar(int availableWidth) {
            m_AvailableWidth = availableWidth;
         }

         private readonly int m_AvailableWidth;

         State IKeyHandler.Handle(ConsoleKeyInfo consoleKeyInfo, State state) {
            Debug.Assert(state != null);
            // handle char
            var keyChar = consoleKeyInfo.KeyChar;
            Debug.Assert(Convert.ToInt32(keyChar) != 0);

            // Don't accept control char!
            if (Char.IsControl(keyChar)) { return state; }

            // When a char is typed, remove selection if any
            state = SelectionHelper.RemoveSelectionIfAny(state);

            // Gather current sub-states
            var lines = state.Lines;
            int x = state.CursorX;
            int y = state.CursorY;
            var currentLine = state.Lines[y];

            // Get the new char!
            if (x < m_AvailableWidth -1) {
               // Just append the new char
               lines[y] = currentLine.Insert(x, keyChar.ToString());
               x++;
            } else {
               // Introduce new line 
               lines.Insert(y + 1, keyChar.ToString());
               x = 1;
               y++;
            }
            return new State(lines, x, y, Location.Null);
         }


         bool IKeyHandler.IsHandlerFor(ConsoleKeyInfo consoleKeyInfo) {
            // default handler!
            return true;
         }
      }
   }
}
