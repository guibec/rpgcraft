using System;
using System.Collections.Generic;
using System.Diagnostics;
using NDepend.CodeModel;

namespace NDepend.PowerTools.SharedUtils {
   public abstract class CodeElementsDisplayerBase<TDisplayInfo> where TDisplayInfo : class {

      protected CodeElementsDisplayerBase() { }

      protected abstract void Display(TDisplayInfo displayInfo);
      protected abstract ICodeElement GetCodeElement(TDisplayInfo displayInfo);
      protected abstract TDisplayInfo GetNext();
      protected virtual void EnterCodeElement(ICodeElement codeElement) {
         Debug.Assert(codeElement != null);
         ConsoleUtils.TryOpenSource(codeElement);
      }

      internal void Display() {
         Console.CursorLeft = 0;

         const int HEIGHT_MINUS = 7;
         var cursorMax = Console.CursorTop + Console.WindowHeight - HEIGHT_MINUS;
         bool showAll = false;
         bool showEnd = false;

         Console.CursorVisible = false;
         try {
            while (true) {
               var displayInfo = GetNext();
               if (displayInfo == null) {
                  showEnd = true;
                  goto ASK_MENU;
               }

               // Display row!
               var cursorTopRow = Console.CursorTop;
               this.Display(displayInfo);
               var nbLines = Console.CursorTop - cursorTopRow;

               var row = new Row(displayInfo, cursorTopRow, nbLines, this.Display);
               m_Rows.Add(row);

               // Ask up/down/enter/all/next/quit? (not too often thanks to cursorMax));
               if (Console.CursorTop < cursorMax) { continue; }

               cursorMax = Console.CursorTop + Console.WindowHeight - HEIGHT_MINUS;
               if (showAll) { continue; }
      ASK_MENU:
               bool showMenu = true;
               var cursorTopMenu = Console.CursorTop;
      ASK_MENU_UP_DOWN_ENTER:
               var choice = ConsoleAskUpDownEnterAllNextQuit(showMenu, showEnd, cursorTopMenu);
               switch (choice) {
                  case UpDownEnterAllNextQuit.Next:
                     goto SHOW_MORE;

                  case UpDownEnterAllNextQuit.All:
                     showAll = true;
                     goto SHOW_MORE;

                  case UpDownEnterAllNextQuit.Up:
                  case UpDownEnterAllNextQuit.Down:
                  case UpDownEnterAllNextQuit.Enter:
                     UpDownEnterChoice(choice);
                     showMenu = false;
                     goto ASK_MENU_UP_DOWN_ENTER;

                  default:
                     // Quit
                     UnselectRow();
                     SetCursorAfterLastRow();
                     return;
               }
       SHOW_MORE:
               UnselectRow();
               SetCursorAfterLastRow();
               continue;
            }
         } finally {
            Console.CursorVisible = true;
         }
      }



      private const ConsoleColor BACK_COLOR_SELECTED = ConsoleColor.DarkGreen;
      private const ConsoleColor BACK_COLOR_REGULAR = ConsoleColor.Black;

      private int? m_RowIndex;

      private void UnselectRow() {
         if (m_RowIndex == null) { return; }
         var row = m_Rows[m_RowIndex.Value];
         row.Display(BACK_COLOR_REGULAR);
         m_RowIndex = null;
      }

      private void SetCursorAfterLastRow() {
         if (m_Rows.Count > 0) {
            var lastRow = m_Rows[m_Rows.Count - 1];
            Console.CursorTop = lastRow.CursorTop + lastRow.NbLines;
         }
         Console.CursorLeft = 0;
      }
      


      private void UpDownEnterChoice(UpDownEnterAllNextQuit choice) {
         Debug.Assert(choice == UpDownEnterAllNextQuit.Up ||
                      choice == UpDownEnterAllNextQuit.Down ||
                      choice == UpDownEnterAllNextQuit.Enter);
         if (m_Rows.Count == 0) { return; }
         var previousRowIndex = m_RowIndex;
         if (m_RowIndex == null) {
            // Select last!
            m_RowIndex = m_Rows.Count - 1;
            m_Rows[m_RowIndex.Value].Display(BACK_COLOR_SELECTED);
            return;
         }

         //
         // Show source file decl
         //
         if (choice == UpDownEnterAllNextQuit.Enter) {
            var rowToShowSource = m_Rows[m_RowIndex.Value];
            var codeElement = GetCodeElement(rowToShowSource.DisplayInfo);
            EnterCodeElement(codeElement);
            return;
         }


         //
         // Up / Down arrow selection
         //
         switch (choice) {
            case UpDownEnterAllNextQuit.Up:
               if (previousRowIndex != null && m_RowIndex.Value != 0) { m_RowIndex--; }
               break;
            case UpDownEnterAllNextQuit.Down:
               if (previousRowIndex != null && m_RowIndex.Value != m_Rows.Count - 1) { m_RowIndex++; }
               break;
         }
         Debug.Assert(m_RowIndex != null);
         // Eventually unselect previous row...
         if (m_RowIndex != previousRowIndex && previousRowIndex != null) {
            m_Rows[previousRowIndex.Value].Display(BACK_COLOR_REGULAR);
         }
         // ... and select current!
         m_Rows[m_RowIndex.Value].Display(BACK_COLOR_SELECTED);
      }




      //
      // Keep a list of rows show to navigate up/down and open source decl!
      //
      private readonly List<Row> m_Rows = new List<Row>();

      private class Row {
         internal Row(TDisplayInfo displayInfo, int cursorTop, int nbLines, Action<TDisplayInfo> displayProc) {
            Debug.Assert(displayInfo != null);
            Debug.Assert(cursorTop >= 0);
            Debug.Assert(nbLines > 0);
            Debug.Assert(displayProc != null);
            m_DisplayInfo = displayInfo;
            m_CursorTop = cursorTop;
            m_NbLines = nbLines;
            m_DisplayProc = displayProc;
         }
         private readonly TDisplayInfo m_DisplayInfo;
         internal TDisplayInfo DisplayInfo { get { return m_DisplayInfo; } }

         private readonly int m_CursorTop;
         internal int CursorTop { get { return m_CursorTop; } }

         private readonly int m_NbLines;
         internal int NbLines { get { return m_NbLines; } }

         private readonly Action<TDisplayInfo> m_DisplayProc;

         internal void Display(ConsoleColor backColor) {
            Console.CursorTop = m_CursorTop;
            Console.CursorLeft = 0;
            ConsoleUtils.ShowNLinesOnConsole(m_NbLines, backColor);

            Console.CursorTop = m_CursorTop;
            Console.CursorLeft = 0;
            Console.BackgroundColor = backColor;
            m_DisplayProc(m_DisplayInfo);

            // Reset cursor to beginning of the row displayed to avoid cursor trace!
            Console.CursorTop = m_CursorTop;
            Console.CursorLeft = 0;
         }
      }


      enum UpDownEnterAllNextQuit {
         All,
         Next,
         Quit,
         Up,
         Down,
         Enter
      }

      private static UpDownEnterAllNextQuit ConsoleAskUpDownEnterAllNextQuit(bool showMenu, bool showEnd, int cursorTop) {
         if (showMenu) {
            Debug.Assert(Console.CursorTop == cursorTop);
            Console.WriteLine();
            Console.CursorLeft = 0;
            Console.BackgroundColor = ConsoleColor.Black;
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Open source decl? Up/Down/Enter -- " + (showEnd ? "" : "Show Next? n  -- Show All? a -- ") + "Stop? any key");
            Console.ForegroundColor = ConsoleColor.White;
         }

         var consoleKeyInfo = Console.ReadKey();

         var keyChar = Char.ToLower(consoleKeyInfo.KeyChar);
         if (!showEnd && (keyChar == 'n' || keyChar == 'a')) {

            // Erase the menu!
            Console.CursorTop = cursorTop;
            ConsoleUtils.ShowNLinesOnConsole(4, ConsoleColor.Black);
            Console.CursorTop = cursorTop;
            // HACK: need to erase a second time, sometime erasing just once is not enought??
            ConsoleUtils.ShowNLinesOnConsole(4, ConsoleColor.Black);
            Console.CursorTop = cursorTop;

            return keyChar == 'n' ? UpDownEnterAllNextQuit.Next : UpDownEnterAllNextQuit.All;
         }

         switch (consoleKeyInfo.Key) {
            case ConsoleKey.DownArrow:
               return UpDownEnterAllNextQuit.Down;
            case ConsoleKey.UpArrow:
               return UpDownEnterAllNextQuit.Up;
            case ConsoleKey.Enter:
               return UpDownEnterAllNextQuit.Enter;
         }

         return UpDownEnterAllNextQuit.Quit;
      }

   }
}
