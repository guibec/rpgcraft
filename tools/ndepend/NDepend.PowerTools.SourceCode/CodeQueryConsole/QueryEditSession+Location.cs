

using System.Collections.Generic;
using System.Diagnostics;

namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {

      // Location coordinate are zero based
      class Location {
         internal Location(int x, int y) {
            m_X = x;
            m_Y = y;
         }

         internal Location(List<string> lines, int position) {
            Debug.Assert(lines != null);
            var index = 0;
            var previousIndex = 0;
            int y;
            for (y = 0; y < lines.Count; y++) {
               index += lines[y].Length;
               if (position <= index || y == lines.Count - 1) { break; }
               index += LINE_FEED.Length;
               previousIndex = index;
            }
            Debug.Assert(position >= previousIndex);
            m_Y = y;
            m_X = position - previousIndex;
         }


         private readonly int m_X;
         internal int X { get { return m_X; } }

         private readonly int m_Y;
         internal int Y { get { return m_Y; } }

         internal readonly static Location Null = null;

         internal int GetPosition(List<string> lines) {
            Debug.Assert(lines != null);
            var position = 0;
            for (var y = 0; y < m_Y; y++) {
               position += lines[y].Length;
               position += LINE_FEED.Length;
            }
            position += m_X;
            return position;
         }


         public static bool operator <(Location locationA, Location locationB) {
            return !AGreatherThanB(locationA, locationB);
         }
         public static bool operator >(Location locationA, Location locationB) {
            return AGreatherThanB(locationA, locationB);
         }
         private static bool AGreatherThanB(Location locationA, Location locationB) {
            Debug.Assert(locationA != null);
            Debug.Assert(locationB != null);
            if (locationA.Y > locationB.Y) { return true; }
            if (locationA.Y < locationB.Y) { return false; }
            return locationA.X > locationB.X;
         }
      }
   }
}
