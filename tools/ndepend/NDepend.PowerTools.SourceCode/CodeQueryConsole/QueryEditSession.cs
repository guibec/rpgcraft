using System;
using System.Collections.Generic;
using System.Diagnostics;
using NDepend.CodeQuery;

namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession : IDisposable {
      

      // LINE_FEED is used by several nesed classes
      private const string LINE_FEED = "\r\n";

      internal QueryEditSession(IPreviousNextStack<string> alreadyEditedQueries) {
         Debug.Assert(alreadyEditedQueries != null);

         Console.CancelKeyPress += this.On_Ctrl_C_Pressed;

         m_AlreadyEditedQueries = alreadyEditedQueries;

         m_ConsoleWriter = new ConsoleWriter();
         
         m_UndoRedoStates = new PreviousNextStack<State>(
            new State[] {} , 
            State.DefaultState,
            (s1,s2) => !s1.IsDifferentThan(s2));

         m_KeyHandlers = new IKeyHandler[] {
               // Order of keyHandlers declaration matters!
               new KeyHandlerCtrl(m_UndoRedoStates),
               new KeyHandlerCursor(ConsoleWriter.AvailableWidth),
               new KeyHandlerChar(ConsoleWriter.AvailableWidth)
            };
      }


      public void Dispose() {
         Console.CancelKeyPress -= this.On_Ctrl_C_Pressed;
      }

      // Notice the special work for handling Ctrl+C pressed (i.e copy selection to clipboard)
      void On_Ctrl_C_Pressed(object sender, ConsoleCancelEventArgs args) {
         KeyHandlerCtrl.HandleCtrlCPressed(m_CurrentState);
      }


      private readonly IPreviousNextStack<State> m_UndoRedoStates;
      private readonly IPreviousNextStack<string> m_AlreadyEditedQueries;
      private readonly ConsoleWriter m_ConsoleWriter;
      private readonly IEnumerable<IKeyHandler> m_KeyHandlers;

      private State m_CurrentState;


      //
      // The 2 QueryEditSession visble methods
      // GetQueryString() and ShowCompilatioErrorsAndThenGetQueryString()
      //
      internal string GetQueryString() {
         m_CurrentState = new State(m_AlreadyEditedQueries.Current, 0,0, Location.Null);
         return EditLoop(null);
      }

      internal string ShowCompilatioErrorsAndThenGetQueryString(IQueryCompiledError queryCompiledError) {
         Debug.Assert(queryCompiledError != null);
         Debug.Assert(m_CurrentState != null);
         return EditLoop(queryCompiledError);
      }

      //
      // The EditLoop: One loop for each key pressed!
      //
      private string EditLoop(IQueryCompiledError queryCompiledError) {
         
         while (true) {
            //
            // Fill m_UndoRedoStates
            //
            m_UndoRedoStates.Append(m_CurrentState);

            //
            // Re-write the query each time a key is pressed
            //
            m_ConsoleWriter.Rewrite(m_CurrentState);

            // Eventually show errors
            if (queryCompiledError != null) {
               // Show errors once...
               m_ConsoleWriter.ShowQueryCompiledErrors(m_CurrentState, queryCompiledError);
               // ... and then set queryCompiledError to null to avoid showing them anymore
               queryCompiledError = null;
            }

            //
            // ReadKey!
            //
            var consoleKeyInfo = Console.ReadKey();

            //
            // Handle special console key
            //
            switch (consoleKeyInfo.Key) {
               case ConsoleKey.F5: // F5 --> Run the query!;
                  Console.CursorVisible = false;
                  m_ConsoleWriter.Rewrite(m_CurrentState);  // <-- need to rewrite the query, else the char where the cursor was is hidden!
                  m_ConsoleWriter.SetCursorPosition(0, m_CurrentState.Lines.Count);
                  Console.CursorLeft = 0;
                  Console.CursorVisible = true;
                  m_AlreadyEditedQueries.AppendButDontClearNext(m_CurrentState.QueryString);
                  return m_CurrentState.QueryString;

               case ConsoleKey.Escape:
                  return null;

               case ConsoleKey.PageUp:
                  m_ConsoleWriter.ClearQueryEdit(m_CurrentState);
                  m_CurrentState = new State(m_AlreadyEditedQueries.GetPrevious(), 0, 0, Location.Null);
                  m_UndoRedoStates.Clear(m_CurrentState);
                  continue;

               case ConsoleKey.PageDown:
                  m_ConsoleWriter.ClearQueryEdit(m_CurrentState);
                  m_CurrentState = new State(m_AlreadyEditedQueries.GetNext(), 0, 0, Location.Null);
                  m_UndoRedoStates.Clear(m_CurrentState);
                  continue;
            }

            //
            // Handle the key inputed with the appropriate keyHandler
            //
            foreach (var keyHandler in m_KeyHandlers) {
               if (!keyHandler.IsHandlerFor(consoleKeyInfo)) { continue; }
               m_CurrentState = keyHandler.Handle(consoleKeyInfo, m_CurrentState);
               break;
            }
            continue;
         }
      }




   }
}
