using System;
using System.Diagnostics;


namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {
      class KeyHandlerCursor : IKeyHandler {


         internal KeyHandlerCursor(int availableWidth) {
            m_AvailableWidth = availableWidth;
         }

         private readonly int m_AvailableWidth;

         State IKeyHandler.Handle(
               ConsoleKeyInfo consoleKeyInfo,
               State state) {
            Debug.Assert(state != null);

            ConsoleKey consoleKey = consoleKeyInfo.Key;
            bool shiftPressed = (consoleKeyInfo.Modifiers & ConsoleModifiers.Shift) != 0;

            //
            // Define the next selectionStart
            //
            Location nextSelectionStart;
            bool selectionHasBeenRemoved;
            state = HandleSelection(
               state, 
               consoleKey,
               shiftPressed, 
               out nextSelectionStart, 
               out selectionHasBeenRemoved);

            //
            // Gather useful states to modify
            //
            int x = state.CursorX;
            int y = state.CursorY;
            var lines = state.Lines;
            var currentLine = lines[y];


            switch (consoleKey) {
               case ConsoleKey.DownArrow:
                  if (y >= lines.Count - 1) { return state; }
                  y++;

                  // re-adjust x if nextLine shorter than currentLine
                  var nextLine = lines[y];
                  if (x > nextLine.Length) {
                     x = nextLine.Length;
                  }
                  break;

               case ConsoleKey.UpArrow:
                  if (y <= 0) { return state; }
                  y--;

                  // re-adjust x if previousLine shorter than currentLine
                  var previousLine = lines[y];
                  if (x > previousLine.Length) {
                     x = previousLine.Length;
                  }
                  break;

               case ConsoleKey.RightArrow:
                  if (x + 1 >= m_AvailableWidth) { return state; }
                  if (x >= currentLine.Length) {
                     if (y >= lines.Count - 1) { return state; }
                     // Go next line beginning
                     y++;
                     x = 0;
                     break;
                  }
                  x++;
                  break;

               case ConsoleKey.LeftArrow:
                  if (x >= m_AvailableWidth) { x = m_AvailableWidth - 1; }
                  if (x == 0) {
                     if (y <= 0) { return state; }
                     // Go previous line end
                     y--;
                     var previousLineLength = lines[y].Length;
                     x = previousLineLength > m_AvailableWidth - 1 ? m_AvailableWidth - 1 : previousLineLength;
                  }
                  x--;
                  break;

               case ConsoleKey.Enter:
                  lines[y] = currentLine.Substring(0, x);
                  var newLine = currentLine.Substring(x, currentLine.Length - x);
                  lines.Insert(y + 1, newLine);
                  x = 0;
                  y++;
                  break;

               case ConsoleKey.Backspace:
                  // Don't execute backspace if select has been removed!
                  if (selectionHasBeenRemoved) { break; }
                  if (x == 0) {
                     if (y == 0) { return state; }
                     y--;
                     x = lines[y].Length;
                     lines[y] += currentLine;
                     lines.RemoveAt(y + 1);
                     break;
                  }
                  lines[y] = currentLine.Substring(0, x - 1) + currentLine.Substring(x, currentLine.Length - x);
                  x--;
                  break;

               case ConsoleKey.End:
                  x = currentLine.Length > m_AvailableWidth -1 ? m_AvailableWidth -1 : currentLine.Length;
                  break;

               case ConsoleKey.Home:
                  x = 0;
                  break;
            }


            return new State(lines, x, y, nextSelectionStart);
         }


         bool IKeyHandler.IsHandlerFor(ConsoleKeyInfo consoleKeyInfo) {
            if (Convert.ToInt32(consoleKeyInfo.KeyChar) == 0) { return true; }

            switch (consoleKeyInfo.Key) {
               case ConsoleKey.Enter:      // ConsoleKey.Enter emit a '\r' char
               case ConsoleKey.Backspace:  // ConsoleKey.Backspace emits a back char
                  return true;
               default:
                  return false;
            }
         }



         private static State HandleSelection(
               State state, 
               ConsoleKey consoleKey,
               bool shiftPressed, 
               out Location selectionStart, 
               out bool selectionHasBeenRemoved) {
            Debug.Assert(state != null);
            selectionStart = Location.Null;
            selectionHasBeenRemoved = false;
            switch(consoleKey) {
                // this 2 keys provoque remove selection if any!
               case ConsoleKey.Enter:
               case ConsoleKey.Backspace:
                  selectionHasBeenRemoved = state.SelectionStart != Location.Null;
                  state = SelectionHelper.RemoveSelectionIfAny(state);
                  break;

               default:
                  selectionStart = state.SelectionStart;
                  if (selectionStart == null && shiftPressed) {
                     selectionStart = new Location(state.CursorX, state.CursorY);
                  }
                  if (selectionStart != null && !shiftPressed) {
                     selectionStart = Location.Null;
                  }
                  break;
            }
            return state;
         }


      }
   }
}
