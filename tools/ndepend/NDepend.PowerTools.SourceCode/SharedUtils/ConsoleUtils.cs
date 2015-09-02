
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using NDepend.CodeModel;

namespace NDepend.PowerTools.SharedUtils {
   static class ConsoleUtils {

      internal static void TryOpenSource(ICodeElement codeElement) {
         Debug.Assert(codeElement != null);
         string failureReason;
         if (!codeElement.TryOpenSource(out failureReason) && failureReason != null) {
            MessageBox.Show(
"Can't open source file decl of {" + codeElement.Name + @"}
Reason:" + failureReason,
               "NDepend.PowerTools", MessageBoxButtons.OK, MessageBoxIcon.Error);
         }
      }


      internal static void TryCompareSource(ISourceFileLine sourceFileLineA, ISourceFileLine sourceFileLineB, string failureReasonIfOneIsNull) {
         string failureReason;
         if (sourceFileLineA == null || sourceFileLineB == null) {
            failureReason = failureReasonIfOneIsNull;
            goto ERROR;
         }

         if (!sourceFileLineA.TryCompareSourceWith(sourceFileLineB, out failureReason) ) {
            goto ERROR;
         }
         return;
   ERROR:
         MessageBox.Show(
@"Can't compare source file decl:
Reason:" + failureReason,
               "NDepend.PowerTools", MessageBoxButtons.OK, MessageBoxIcon.Error);
      }

      internal static void ShowConsoleMemberFullNamePrefixedWithAssembly(IMember member) {
         Debug.Assert(member != null);
         Console.ForegroundColor = ConsoleColor.DarkGray;
         Console.Write(member.ParentAssembly.Name+ "::");

         ShowConsoleMemberFullName(member);
      }


      internal static void ShowConsoleMemberFullName(IMember member) {
         Debug.Assert(member != null);
         Console.ForegroundColor = ConsoleColor.DarkMagenta;
         Console.Write(member.ParentNamespace.Name);

         if (member.ParentType != null) { // Can happen for non nested type
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.Write(".");
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.Write( member.ParentType.Name);
         }
         Console.ForegroundColor = ConsoleColor.DarkGray;
         Console.Write(".");
         Console.ForegroundColor = ConsoleColor.White;
         Console.WriteLine(member.Name);
      }



      internal static void ShowNLinesOnConsole(int n, ConsoleColor consoleColor) {
         Debug.Assert(n >= 0);
         var initialBackColor = Console.BackgroundColor;
         Console.BackgroundColor = consoleColor;
         var strEmpty = new string(' ', Console.WindowWidth);
         for (var i = 0; i < n; i++) {
            Console.Write(strEmpty);
         }
         Console.BackgroundColor = initialBackColor;
      }



      internal static AllNextQuit ConsoleAskShowNextAllStop() {
         var cursorTop = Console.CursorTop;
         Console.WriteLine();
         Console.CursorLeft = 0;
         Console.BackgroundColor = ConsoleColor.Black;
         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine("Show Next?  n     Show All?  a     Stop? any key");
         Console.ForegroundColor = ConsoleColor.White;

         var keyChar = Char.ToLower(Console.ReadKey().KeyChar);

         if (keyChar == 'n' || keyChar == 'a') {
            Console.CursorTop = cursorTop;
            ShowNLinesOnConsole(4, ConsoleColor.Black);
            Console.CursorTop = cursorTop;
            // HACK: need to erase a second time, sometime erasing just once is not enought??
            ShowNLinesOnConsole(4, ConsoleColor.Black);
            Console.CursorTop = cursorTop;
            return keyChar == 'n' ? AllNextQuit.Next : AllNextQuit.All;
         }

         return AllNextQuit.Quit;
      }


      [DllImport("kernel32.dll")]
      private static extern IntPtr GetConsoleWindow();

      internal static IntPtr MainWindowHandle {
         get {
            return GetConsoleWindow();
         }
      }
   }

   enum AllNextQuit {
      All,
      Next,
      Quit
   }

}
