using System;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;


namespace NDepend.PowerTools.CodeQueryConsole {

   partial class QueryEditSession {
      class KeyHandlerCtrl : IKeyHandler {

         internal KeyHandlerCtrl(IPreviousNextStack<State> undoRedoStates) {
            Debug.Assert(undoRedoStates != null);
            m_UndoRedoStates = undoRedoStates;
         }
         private readonly IPreviousNextStack<State> m_UndoRedoStates;


         State IKeyHandler.Handle(ConsoleKeyInfo consoleKeyInfo, State state) {
            Debug.Assert(state != null);
            var lines = state.Lines;

            switch (consoleKeyInfo.Key) {
               case ConsoleKey.A: // select all
                  var cursorY = lines.Count - 1;
                  var cursorX = lines[cursorY].Length;
                  return new State(
                     lines,
                     cursorX,
                     cursorY,
                     new Location(0,0));

               //
               // Clipboard  
               //    Notice, Clipboard works because the current thread is STAThread.
               //    Notice, Ctrl+C is handled by another thread thanks to the Console.
               //
               case ConsoleKey.X: // cut  
                  CopySelectionToClipboard(state);
                  return SelectionHelper.RemoveSelectionIfAny(state);
                  
               case ConsoleKey.V: // paste
                  var pasteText = Clipboard.GetText();
                  if (string.IsNullOrEmpty(pasteText)) { break; }
                  state = SelectionHelper.RemoveSelectionIfAny(state);
                  var queryString = state.QueryString;
                  var cursorPosition = new Location(state.CursorX, state.CursorY).GetPosition(state.Lines);
                  Debug.Assert(cursorPosition <= queryString.Length);
                  var newQueryString = queryString.Insert(cursorPosition, pasteText);

                  // Need stateTmp to get locationAtEndPaste
                  var stateTmp = new State(newQueryString, 0, 0, Location.Null);
                  var cursorPositionAtEndPaste = cursorPosition + pasteText.Length;
                  var locationAtEndPaste = new Location(stateTmp.Lines, cursorPositionAtEndPaste);
                  return new State(newQueryString, locationAtEndPaste.X, locationAtEndPaste.Y, Location.Null);

                  

               //
               // Undo/Redo
               //
               case ConsoleKey.Y: // redo
                  return m_UndoRedoStates.GetNext();
                  
               case ConsoleKey.Z: // undo
                  return m_UndoRedoStates.GetPrevious();
            }

            return state;
         }

 

         

         bool IKeyHandler.IsHandlerFor(ConsoleKeyInfo consoleKeyInfo) {
            var ctrlPressed = (consoleKeyInfo.Modifiers & ConsoleModifiers.Control) != 0;
            if (!ctrlPressed) { return false; }

            switch (consoleKeyInfo.Key) {
               case ConsoleKey.A:
               case ConsoleKey.X:
               case ConsoleKey.V:
               case ConsoleKey.C:
               case ConsoleKey.Y:
               case ConsoleKey.Z:
                  return true;
               default:
                  return false;
            }
         }




         private static void CopySelectionToClipboard(State state) {
            Debug.Assert(state != null);
            int beginPosition, endPosition;
            if (!SelectionHelper.TryGetBeginEndPositions(state, out beginPosition, out endPosition)) {
               return;
            }
            var selectionText = state.QueryString.Substring(beginPosition, endPosition - beginPosition);
            Clipboard.SetText(selectionText);
         }



         internal static void HandleCtrlCPressed(State state) {
            Debug.Assert(state != null);

            // Worker thread is not STA, hence this code
            // http://stackoverflow.com/questions/518701/clipboard-gettext-returns-null-empty-string
            var staThread = new Thread(
                new ThreadStart(delegate {
               try {
                  CopySelectionToClipboard(state);
               } catch {
               }
            }));
            staThread.SetApartmentState(ApartmentState.STA);
            staThread.Start();
            staThread.Join();
         }
      }

   }
}
