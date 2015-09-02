using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {


      class State {

         internal readonly static State DefaultState = new State("", 0, 0, Location.Null);

         internal State(List<string> lines, int cursorX, int cursorY, Location selectionStart) {
            Debug.Assert(lines != null);
            Debug.Assert(lines.Count > 0);
            // selectionStart can be null
            m_CursorLocation = new Location(cursorX, cursorY);
            var sb = new StringBuilder();
            for (var i = 0; i < lines.Count; i++) {
               sb.Append(lines[i]);
               if (i == lines.Count - 1) { break; }
               sb.Append(LINE_FEED);
            }
            m_QueryString =  sb.ToString();
            m_SelectionStart = selectionStart;
         }

         internal State(string queryString, int cursorX, int cursorY, Location selectionStart) {
            Debug.Assert(queryString != null);
            // selectionStart can be null
            m_CursorLocation = new Location(cursorX, cursorY);
            m_QueryString = queryString;
            m_SelectionStart = selectionStart;
         }

         private readonly string m_QueryString;
         internal string QueryString { get { return m_QueryString; } }

         internal List<string> Lines {
            get {
               var text = m_QueryString.Replace(LINE_FEED, "\n");
               var lines = new List<string>(text.Split(new[] { '\n' }));
               return lines;
            }
         }

         private readonly Location m_CursorLocation;
         internal int CursorX { get { return m_CursorLocation.X; } }
         internal int CursorY { get { return m_CursorLocation.Y; } }

         private readonly Location m_SelectionStart;
         internal Location SelectionStart { get { return m_SelectionStart; } }

         // 2 states are considered different if the queryString or the selection is different.
         // In case there is no selection, the cursor location doesn't matter.
         internal bool IsDifferentThan(State otherState) {
            Debug.Assert(otherState != null);
            var lines = this.Lines;
            return otherState.m_QueryString != this.m_QueryString ||
                   (otherState.m_SelectionStart != null) != (this.m_SelectionStart != null) ||

                   (otherState.m_SelectionStart != null && this.m_SelectionStart != null &&
                    (
                     ( otherState.m_SelectionStart.GetPosition(lines) != this.m_SelectionStart.GetPosition(lines) ) ||
                     ( otherState.m_CursorLocation.GetPosition(lines) != this.m_CursorLocation.GetPosition(lines) )
                    )
                   );
         }
      }
   }
}
