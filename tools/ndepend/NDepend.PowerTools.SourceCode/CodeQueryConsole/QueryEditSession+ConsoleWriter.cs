using System;
using System.Collections.Generic;
using System.Diagnostics;
using NDepend.CodeQuery;
using NDepend.PowerTools.SharedUtils;


namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {
      partial class ConsoleWriter {

         internal ConsoleWriter() {
            m_QueryEditionTop = Console.CursorTop;
         }

         private const string PROMPTER = "> ";  

         private const ConsoleColor BACK_COLOR = ConsoleColor.White;
         private const ConsoleColor FORE_COLOR = ConsoleColor.Black;
         private const ConsoleColor KEYWORD_FORE_COLOR = ConsoleColor.Blue;
         private const ConsoleColor COMMENT_FORE_COLOR = ConsoleColor.DarkGreen;
         private const ConsoleColor PROMPTER_FORE_COLOR = ConsoleColor.DarkGray;
         private const ConsoleColor PROMPTER_BACK_COLOR = ConsoleColor.Gray;
         private const ConsoleColor QUERY_NAME_FORE_COLOR = ConsoleColor.Black;
         private const ConsoleColor QUERY_NAME_BACK_COLOR = ConsoleColor.Green;

         private const ConsoleColor SELECTION_BACK_COLOR = ConsoleColor.Blue;
         private const ConsoleColor SELECTION_FORE_COLOR = ConsoleColor.White;

         private readonly int m_QueryEditionTop;

         internal static int AvailableWidth { get { return Console.WindowWidth - PROMPTER.Length; } }

         // represent the maximum # of lines edited during this session
         private int m_MaxNbLines = 0;

         internal void ClearQueryEdit(State state) {
            Debug.Assert(state != null);
            Console.CursorTop = m_QueryEditionTop;
            Console.CursorLeft = 0;
            var nbLines = state.Lines.Count;
            Debug.Assert(m_MaxNbLines >= nbLines);
            ConsoleUtils.ShowNLinesOnConsole(nbLines, BACK_COLOR);
            ConsoleUtils.ShowNLinesOnConsole(m_MaxNbLines - nbLines, ConsoleColor.Black);
         }



         // x,y are zero based coordinate in query string
         internal void SetCursorPosition(int x, int y) {
            var availableWidth = AvailableWidth;
            Console.CursorLeft = PROMPTER.Length + (x >= availableWidth ? availableWidth -1 : x);
            Console.CursorTop = m_QueryEditionTop + y;
         }
         private void GetCursorPosition(out int x, out int y) {
            x = Console.CursorLeft - PROMPTER.Length;
            y = Console.CursorTop - m_QueryEditionTop;
         }




         #region Rewrite
         //-----------------------------------------------------------------
         //
         // Rewrite
         //
         //-----------------------------------------------------------------
         internal void Rewrite(State state) {
            Debug.Assert(state != null);
            Console.BackgroundColor = BACK_COLOR;
            Console.ForegroundColor = FORE_COLOR;
            Console.CursorVisible = false;
            var lines = state.Lines;
            var nbLines = lines.Count;

            var highlightersPerLines = SyntaxHighlighter.Go(lines);
            Debug.Assert(highlightersPerLines.Length == nbLines);


            m_MaxNbLines = nbLines > m_MaxNbLines ? nbLines : m_MaxNbLines;
            Console.CursorTop = m_QueryEditionTop - 1;
            Console.CursorLeft = 0;
            for (var i = 0; i < nbLines; i++) {
               var line = lines[i];
               var highlights = highlightersPerLines[i];
               ShowNewLine(line, highlights);
            }

            Console.CursorTop = m_QueryEditionTop + nbLines;
            Console.CursorLeft = 0;
            ConsoleUtils.ShowNLinesOnConsole(m_MaxNbLines - nbLines, ConsoleColor.Black);

            ShowSelection(state);

            this.SetCursorPosition(state.CursorX, state.CursorY);
            Console.CursorVisible = true;
         }

         private void ShowNewLine(string line, IEnumerable<Highlight> highlights) {
            Debug.Assert(line != null);
            Debug.Assert(highlights != null);

            int x, y;
            GetCursorPosition(out x, out y);
            Console.WriteLine();

            // Write prompter
            Console.BackgroundColor = PROMPTER_BACK_COLOR;
            Console.ForegroundColor = PROMPTER_FORE_COLOR;
            Console.Write(PROMPTER); 

            // Write Highlighters
            var availableWidth = AvailableWidth;
            foreach (var highlighter in highlights) {
               if (highlighter.StartPosition > availableWidth) { continue; }
               var endPosition = highlighter.StartPosition + highlighter.Length;
               if (endPosition > availableWidth) { endPosition = availableWidth; }
               Console.ForegroundColor = highlighter.ForegroundColor;
               Console.BackgroundColor = highlighter.BackgroundColor;
               var text = line.Substring(highlighter.StartPosition, endPosition - highlighter.StartPosition);
               Console.Write(text);
            }

            // Eventually fill the rest of the line
            var lineLength = line.Length;
            if (lineLength < availableWidth) {
               Console.BackgroundColor = BACK_COLOR;
               Console.Write(new string(' ', availableWidth - lineLength));
            }
            SetCursorPosition(0, y + 1);
         }
         #endregion Rewrite


         #region Show selection
         //-----------------------------------------------------------------
         //
         // Show selection
         //
         //-----------------------------------------------------------------
         private void ShowSelection(State state) {
            Debug.Assert(state != null);

            // Obtain selection beginLocation and endLocation
            Location beginLocation, endLocation;
            if (!SelectionHelper.TryGetBeginEndLocations(state, out beginLocation, out endLocation)) {
               return;
            }

            // Show selection
            Console.ForegroundColor = SELECTION_FORE_COLOR;
            Console.BackgroundColor = SELECTION_BACK_COLOR;

            int x = beginLocation.X;
            for (var y = beginLocation.Y; y <= endLocation.Y; y++) {
               var line = state.Lines[y];
               var endX = (y < endLocation.Y) ? line.Length : endLocation.X;
               if (endX - x > AvailableWidth) { endX = AvailableWidth + x; }
               var lineSelection = line.Substring(x, endX - x);
               this.SetCursorPosition(x, y);
               Console.Write(lineSelection);
               x = 0;
            }
         }
         #endregion Show selection



         #region Show query compilation error
         //-----------------------------------------------------------------
         //
         // Show query compilation error
         //
         //-----------------------------------------------------------------
         internal void ShowQueryCompiledErrors(State state, IQueryCompiledError queryCompiledError) {
            Debug.Assert(state != null);
            Debug.Assert(queryCompiledError != null);

            //
            // Write errors descriptions
            //
            Console.ForegroundColor = ConsoleColor.Red;
            Console.BackgroundColor = ConsoleColor.Black;
            var lines = state.Lines;
            Console.CursorTop = m_QueryEditionTop + lines.Count;
            Console.CursorLeft = 0;
            var nbErrors = queryCompiledError.Errors.Count;
            Console.WriteLine(nbErrors + " compilation error" + (nbErrors > 1 ? "s" : ""));
            foreach (var error in queryCompiledError.Errors) {
               Console.WriteLine("  " + error.Description);
            }
            var actualY = Console.CursorTop - m_QueryEditionTop;
            m_MaxNbLines = actualY > m_MaxNbLines ? actualY : m_MaxNbLines;

            //
            // Highlight errors on query edited!
            //
            Console.BackgroundColor = ConsoleColor.Red;
            Console.ForegroundColor = ConsoleColor.White;
            var queryString = state.QueryString;
            var consoleWidth = Console.WindowWidth;
            foreach (var error in queryCompiledError.Errors) {
               var startPos = error.SubStringStartPos;
               var length = error.SubStringLength;
               Debug.Assert(startPos + length <= queryString.Length);
               var location = new Location(lines, startPos);
               length = length > AvailableWidth ? AvailableWidth : length;
               var errorString = queryString.Substring(startPos, length);

               // An error can spawn several lines!
               errorString = errorString.Replace(LINE_FEED, "\n");
               var errorLines = errorString.Split(new [] {'\n'});
               var x = PROMPTER.Length + location.X;
               for (var y = 0; y < errorLines.Length; y++) {
                  Console.CursorTop = m_QueryEditionTop + location.Y + y;
                  var errorLine = errorLines[y];
                  if (x + errorLine.Length < consoleWidth) {
                     Console.CursorLeft = x;
                     Console.Write(errorLine);
                  }
                  x = PROMPTER.Length;
               }
               Console.CursorTop = m_QueryEditionTop;
               Console.CursorLeft = PROMPTER.Length;
            }
            Console.BackgroundColor = BACK_COLOR;
            Console.ForegroundColor = FORE_COLOR;

            SetCursorPosition(state.CursorX, state.CursorY);
         }


         #endregion Show query cmpilation error

      }



   }
}
