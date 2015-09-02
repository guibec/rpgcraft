using System.Diagnostics;
using NDepend.CodeModel;

namespace NDepend.PowerTools.SearchTypesByName {
   class ResultSelector {
      internal ResultSelector(ResultDisplayer resultDisplayer) {
         Debug.Assert(resultDisplayer != null);
         m_ResultDisplayer = resultDisplayer;
         m_ResultDisplayer.OnTypesDisplayedChanged += this.OnTypesDisplayedChanged;
      }

      private readonly ResultDisplayer m_ResultDisplayer;

      private int m_Index = 0;
      private IType[] m_TypesDisplayed = new IType[0];

      private void OnTypesDisplayedChanged(IType[] typesDisplayed) {
         Debug.Assert(typesDisplayed != null);
         m_Index = 0;
         m_TypesDisplayed = typesDisplayed;
         if (m_TypesDisplayed.Length == 0) { return; }
         Select(m_Index);
      }

      internal void UpArrow() {
         if (m_TypesDisplayed.Length == 0) { return; }
         if (m_Index > 0) {
            UnSelect(m_Index);  
            m_Index--;
         }
         Select(m_Index);
      }

      internal void DownArrow() {
         if (m_TypesDisplayed.Length == 0) { return; }
         if (m_Index < m_TypesDisplayed.Length - 1) {
            UnSelect(m_Index);
            m_Index++;
         }
         Select(m_Index);
      }

      internal void Enter() {
         if (m_TypesDisplayed.Length == 0) { return; }
         Debug.Assert(m_Index >= 0);
         Debug.Assert(m_Index <= m_TypesDisplayed.Length);
         m_TypesDisplayed[m_Index].TryOpenSource();
      }


      private void Select(int index) {
         Debug.Assert(index >= 0);
         Debug.Assert(index <= m_TypesDisplayed.Length);
         Debug.Assert(m_TypesDisplayed.Length > 0);
         m_ResultDisplayer.DisplayTypeName(m_TypesDisplayed[index], ResultDisplayer.Select.Yes);
      }

      private void UnSelect(int index) {
         Debug.Assert(index >= 0);
         Debug.Assert(index <= m_TypesDisplayed.Length);
         Debug.Assert(m_TypesDisplayed.Length > 0);
         m_ResultDisplayer.DisplayTypeName(m_TypesDisplayed[index], ResultDisplayer.Select.No);
      }

   }
}
