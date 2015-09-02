using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.CodeQuery;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.CodeQueryConsole {
   internal class CodeElementsDisplayer : CodeElementsDisplayerBase<RecordBase> {

      internal CodeElementsDisplayer(IEnumerable<RecordBase> records, int[] columnsWidth) {
         Debug.Assert(records != null);
         Debug.Assert(columnsWidth != null);
         m_Records = records.ToArray();
         m_ColumnsWidth = columnsWidth;
      }

      private readonly IList<RecordBase> m_Records;
      private readonly int[] m_ColumnsWidth;
      private int m_Index;

      protected override RecordBase GetNext() {
         if (m_Index == m_Records.Count) { return null; }
         var record = m_Records[m_Index];
         m_Index++;
         return record;
      }


      protected override void Display(RecordBase record) {
         Debug.Assert(record != null);
         var stringsColored = QueryExecutionResultDisplayer.FormatCellsContentFromRecord(record);
         QueryExecutionResultDisplayer.DisplayRow(m_ColumnsWidth, stringsColored);
         QueryExecutionResultDisplayer.WriteSeparatorLine(m_ColumnsWidth);
      }

      protected override ICodeElement GetCodeElement(RecordBase record) {
         Debug.Assert(record != null);
         var codeElement = record[0].m_UntypedValue as ICodeElement;
         Debug.Assert(codeElement != null);
         return codeElement;
      }




   }
}
