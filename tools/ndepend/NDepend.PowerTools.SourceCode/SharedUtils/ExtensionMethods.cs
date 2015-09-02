
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace NDepend.PowerTools.SharedUtils {
   static class ExtensionMethods {


      internal static IEnumerable<T> ToEnumerable<T>(this T obj) {
         return new[] { obj };
      }

      internal static string Format1000(this int i) {
         return FormatStringWith1000Spaces(i.ToString());
      }
      internal static string Format1000(this long? i) {
         return FormatStringWith1000Spaces(i.Value.ToString());
      }
      private static string FormatStringWith1000Spaces(string s) {
         Debug.Assert(s != null);
         var sb = new StringBuilder();
         while (true) {
            if (s.Length <= 3) {
               sb.Append(s);
               break;
            }
            var currentSectionLength = s.Length % 3;
            if (currentSectionLength == 0) { currentSectionLength = 3; }
            Debug.Assert(currentSectionLength >= 1);
            Debug.Assert(currentSectionLength <= 3);
            sb.Append(s.Substring(0, currentSectionLength));
            sb.Append(" ");
            s = s.Substring(currentSectionLength, s.Length - currentSectionLength);
         }
         return sb.ToString();
      }
   }
}
