using System.Diagnostics;

namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {

      static class SelectionHelper {

         internal static State RemoveSelectionIfAny(State state) {
            Debug.Assert(state != null);

            // Obtain selection beginPosition and endPosition
            int beginPosition, endPosition;
            if (!TryGetBeginEndPositions(state, out beginPosition, out endPosition)) {
               return state;
            }
            var queryString = state.QueryString.Remove(beginPosition, endPosition - beginPosition);
            var beginLocation = new Location(state.Lines, beginPosition);
            return new State(queryString, beginLocation.X, beginLocation.Y, Location.Null);
         }



         internal static bool TryGetBeginEndPositions(State state, out int beginPosition, out int endPosition) {
            Debug.Assert(state != null);
            Location beginLocation, endLocation;
            if (!TryGetBeginEndLocations(state, out beginLocation, out endLocation)) {
               beginPosition = -1;
               endPosition = -1;
               return false;
            }
            beginPosition = beginLocation.GetPosition(state.Lines);
            endPosition = endLocation.GetPosition(state.Lines);
            // if endIndex at the end of a line, also capture the line feed
            if (state.QueryString.Length >= endPosition + LINE_FEED.Length &&
                state.QueryString.Substring(endPosition, LINE_FEED.Length) == LINE_FEED) {
               endPosition += LINE_FEED.Length;
            }
            return true;
         }


         internal static bool TryGetBeginEndLocations(State state, out Location beginLocation, out Location endLocation) {
            Debug.Assert(state != null);
            var selectionStart = state.SelectionStart;
            var cursorLocation = new Location(state.CursorX, state.CursorY);
            if (selectionStart == Location.Null) {
               beginLocation = null;
               endLocation = null;
               return false;
            }

            if (cursorLocation < selectionStart) {
               beginLocation = cursorLocation;
               endLocation = selectionStart;
               return true;
            }

            beginLocation = selectionStart;
            endLocation = cursorLocation;
            // special case where a whole line is selected but not the next line
            if (endLocation.X == 0 && endLocation.Y > 0) {
               endLocation = new Location(state.Lines[endLocation.Y - 1].Length, endLocation.Y - 1);
            }
            return true;
         }
      }
   }
}
