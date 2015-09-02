using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.CodeQuery;
using NDepend.Helpers;

namespace NDepend.PowerTools.CodeQueryConsole {

   static class QueryExecutionResultDisplayer {
      internal static void Go(IQueryExecutionSuccessResult result) {
         Debug.Assert(result != null);

         //
         // treat scalar value case
         //
         if (result.IsSingleScalarResult) {
            Console.ForegroundColor = ConsoleColor.White;
            var singleScalarValue = result.SingleScalarValue;
            Console.WriteLine("Scalar result: " + (singleScalarValue != null ? singleScalarValue.Value.ToString() : "null"));
            return;
         }

         //
         // Show query result summary
         //
         if (result.IsARuleViolated) {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Warning: The rule is violated : " + result.Description);
         } else {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("" + result.Description);
         }
         Console.WriteLine("");
         if(result.Records.Count == 0) { return; }

         var columnsWidth = GetColumnsWidth(result.ColumnsNames);

         //
         // Write header
         //
         WriteSeparatorLine(columnsWidth);
         var headerRow = result.ColumnsNames.Select(columnName => new List<StringColored> { new StringColored(columnName, ConsoleColor.White)}).ToArray();
         DisplayRow(columnsWidth, headerRow);
         WriteSeparatorLine(columnsWidth);

         //
         // Write rows, and let select code element and show source file decl!
         //
         var codeElementsDisplayer = new CodeElementsDisplayer(result.Records, columnsWidth);
         codeElementsDisplayer.Display();
      }







      #region format cells content
      //---------------------------------------------------------------------------
      //
      // format cells content
      //
      //---------------------------------------------------------------------------
   
      internal static List<StringColored>[] FormatCellsContentFromRecord(RecordBase record) {
         Debug.Assert(record != null);
         var cellsContent = new List<StringColored>[record.RecordArity];

         cellsContent[0] = FormatFirstCellContent(record[0]);

         for (var i = 1; i < record.RecordArity; i++) {
            cellsContent[i] = FormatNonFirstCellContent(record[i]);
         }
         return cellsContent;
      }


      private const ConsoleColor ASSEMBLY_PARENT_COLOR = ConsoleColor.DarkGray;
      private const ConsoleColor PARENT_PARENT_COLOR = ConsoleColor.DarkMagenta;
      private const ConsoleColor PARENT_COLOR = ConsoleColor.Magenta;
      private const ConsoleColor MAIN_COLOR = ConsoleColor.White;

      private static List<StringColored> FormatFirstCellContent(RecordCellValue recordCellValue) {
         Debug.Assert(recordCellValue != null);
         var list = new List<StringColored>();

         switch (recordCellValue.m_RecordCellType) {
            case RecordCellType.Method:
            case RecordCellType.Field:
               var member = recordCellValue.m_UntypedValue as IMember;
               Debug.Assert(member != null);
               list.Add(new StringColored(member.ParentAssembly.Name + "::", ASSEMBLY_PARENT_COLOR));
               list.Add(new StringColored(member.ParentNamespace.Name + ".", PARENT_PARENT_COLOR));
               list.Add(new StringColored(member.ParentType.Name + ".", PARENT_COLOR));
               list.Add(new StringColored(member.Name, MAIN_COLOR));
               break;

            case RecordCellType.Type:
               var type = recordCellValue.m_UntypedValue as IType;
               Debug.Assert(type != null);
               list.Add(new StringColored(type.ParentAssembly.Name + "::", ASSEMBLY_PARENT_COLOR));
               list.Add(new StringColored(type.ParentNamespace.Name + ".", PARENT_COLOR));
               list.Add(new StringColored(type.Name, MAIN_COLOR));
               break;

            case RecordCellType.Namespace:
               var @namespace = recordCellValue.m_UntypedValue as INamespace;
               Debug.Assert(@namespace != null);
               list.Add(new StringColored(@namespace.ParentAssembly.Name + "::", ASSEMBLY_PARENT_COLOR));
               list.Add(new StringColored(@namespace.Name, MAIN_COLOR));
               break;

            default:
               Debug.Assert(recordCellValue.m_RecordCellType == RecordCellType.Assembly);
               var assembly = recordCellValue.m_UntypedValue as IAssembly;
               Debug.Assert(assembly != null);
               list.Add(new StringColored(assembly.Name, MAIN_COLOR));
               break;
         }
         return list;
      }

      private static List<StringColored> FormatNonFirstCellContent(RecordCellValue recordCellValue) {
         Debug.Assert(recordCellValue != null);

         string content;
         switch(recordCellValue.m_RecordCellType) {
            case RecordCellType.Methods: content = (recordCellValue.m_UntypedValue as IEnumerable<IMethod>).Count() + " methods"; break;
            case RecordCellType.Fields: content = (recordCellValue.m_UntypedValue as IEnumerable<IField>).Count() + " fields"; break;
            case RecordCellType.Types: content = (recordCellValue.m_UntypedValue as IEnumerable<IType>).Count() + " types"; break;
            case RecordCellType.Namespaces: content = (recordCellValue.m_UntypedValue as IEnumerable<INamespace>).Count() + " namespaces"; break;
            case RecordCellType.Assemblies: content = (recordCellValue.m_UntypedValue as IEnumerable<IAssembly>).Count() + " assemblies"; break;

            case RecordCellType.Method:
            case RecordCellType.Field:
            case RecordCellType.Type:
            case RecordCellType.Namespace:
            case RecordCellType.Assembly:
               content = (recordCellValue.m_UntypedValue as ICodeElement).Name;
               break;

            default:
               content = recordCellValue.m_UntypedValue != null ? recordCellValue.m_UntypedValue.ToString() : @"N\A";
               break;
         }

         
         return new List<StringColored> { new StringColored(content, ConsoleColor.White) };
      }
      #endregion format cells content









      #region Display rows and cells
      //---------------------------------------------------------------------------
      //
      // Display rows and cells
      //
      //---------------------------------------------------------------------------
      internal static void DisplayRow(int[] columnsWidth, List<StringColored>[] cellsContent) {
         Debug.Assert(columnsWidth != null);
         Debug.Assert(cellsContent != null);
         Debug.Assert(columnsWidth.Length == cellsContent.Length);
         Console.CursorLeft = 0;

         var cursorTop = Console.CursorTop;

         //
         // Make sure we have at least 50 free buffer lines 
         // else a OutOfRange buffer exception might be thrown
         // while displaying a row 
         //
         var bufferHeightRequested = cursorTop + 50;
         if (Console.BufferHeight < bufferHeightRequested) {
            Console.BufferHeight = bufferHeightRequested;
         }


         //
         // write each cell
         //
         var nbLinesToDisplayRow = 1;
         var cursortLeft = 0;
         for (var i = 0; i < columnsWidth.Length; i++) {
            Console.CursorTop = cursorTop;
            Console.CursorLeft = cursortLeft;
            var nbLinesToDisplayCell = DisplayCell(columnsWidth[i], cellsContent[i]);
            if (nbLinesToDisplayCell > nbLinesToDisplayRow) {
               nbLinesToDisplayRow = nbLinesToDisplayCell;
            }
            cursortLeft += columnsWidth[i] + CELL_SEPARATOR.Length;
         }


         //
         // write cell separator
         //
         Console.ForegroundColor = SEPARATOR_COLOR;
         cursortLeft = 0;
         for (var i = 0; i < columnsWidth.Length - 1; i++) {
            var width = columnsWidth[i];
            cursortLeft += width;
            Console.CursorTop = cursorTop;
            for (var j = 0; j < nbLinesToDisplayRow; j++) {
               Console.CursorLeft = cursortLeft;
               Console.CursorTop = cursorTop + j;
               // BUG WORKAROUND: Don't write space, it can result in a buggy overlapping effect!
               //Console.Write(CELL_SEPARATOR);
               foreach (var c in CELL_SEPARATOR.ToCharArray()) {
                  if (c == ' ') {
                     Console.CursorLeft++;
                     continue;
                  }
                  Console.Write(c);
               }
            }
            cursortLeft += CELL_SEPARATOR.Length;
         }


         Console.CursorTop = cursorTop + nbLinesToDisplayRow;
      }

      private static int DisplayCell(int width, List<StringColored> list) {
         Debug.Assert(width > 0);
         Debug.Assert(list != null);
         Debug.Assert(list.Count >= 1);
         var left = Console.CursorLeft;
         var top = Console.CursorTop;
         foreach (var stringColored in list) {
            Console.ForegroundColor = stringColored.Color;
            var @string = stringColored.String;
            int stringIndex = 0;
            var widthAvailableOnLine = width - (Console.CursorLeft - left);
            while (@string.Length - stringIndex >= widthAvailableOnLine) {
               if (widthAvailableOnLine > 0 && @string.Length < width) {
                  // Case where we have to break line, but the current string can fit in width
                  // better break line directly!
                  Console.CursorTop++;
                  Console.CursorLeft = left;
                  break;
               }
               Console.Write(@string.Substring(stringIndex, widthAvailableOnLine));
               stringIndex += widthAvailableOnLine;
               Console.CursorTop++;
               Console.CursorLeft = left;
               widthAvailableOnLine = width;
            }
            Console.Write(@string.Substring(stringIndex, @string.Length - stringIndex));
         }
         return Console.CursorTop - top + 1;
      }
      #endregion Display rows and cells





      #region Helpers
      //---------------------------------------------------------------------------
      //
      // Helpers
      //
      //---------------------------------------------------------------------------
      private const string LINE_SEPARATOR = "-+-";
      private const string CELL_SEPARATOR = " | ";
      private const ConsoleColor SEPARATOR_COLOR = ConsoleColor.DarkGray;

      internal static void WriteSeparatorLine(int[] columnsWidth) {
         Debug.Assert(columnsWidth != null);
         Console.ForegroundColor = SEPARATOR_COLOR;
         Console.CursorLeft = 0;
         for (var i = 0; i < columnsWidth.Length; i++) {
            var width = columnsWidth[i];
            Console.Write(new string('-', width));
            if (i != columnsWidth.Length - 1) {
               Console.Write(LINE_SEPARATOR);
            }
         }
         Console.WriteLine();
      }

      private static int[] GetColumnsWidth(IReadOnlyCollection<string> columnsNames) {
         Debug.Assert(columnsNames != null);
         var nbColumns = columnsNames.Count;
         Debug.Assert(nbColumns >= 1);

         // -1 to avoid adding unwanted new lines
         var consoleWidth = Console.WindowWidth -1;

         //
         // Case just one column
         //
         if (nbColumns == 1) {
            return new [] { consoleWidth };
         }

         //
         // Several columns case
         //
         // The first column that displays code elements names
         // is FIRST_COLUMN_WIDTH larger than any other column.
         const int FIRST_COLUMN_WIDTH = 5;
         var columnsWidth = new int[nbColumns];
         var consoleWidthWithoutOneCharSeparators = consoleWidth - (nbColumns - 1) * CELL_SEPARATOR.Length;
         columnsWidth[0] = consoleWidthWithoutOneCharSeparators * FIRST_COLUMN_WIDTH / (FIRST_COLUMN_WIDTH + nbColumns - 1);

         // All columns after the first one share a similar width.
         var nonFirstColumnWidth = consoleWidthWithoutOneCharSeparators / (FIRST_COLUMN_WIDTH + nbColumns - 1);
         for (var i = 1; i < nbColumns; i++) {
            columnsWidth[i] = nonFirstColumnWidth;
         }
         return columnsWidth;
      }
      #endregion Helpers

   }
}
