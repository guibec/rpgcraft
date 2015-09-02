using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace NDepend.PowerTools.CodeQueryConsole {

   interface IPreviousNextStack<T> where T : class {
      T GetPrevious();
      T GetNext();
      T Current { get; }
      void Append(T item);
      void AppendButDontClearNext(T item);
      void Clear(T defaultItem);
   }

   // Handle the undo / redo pattern
   // Used both for previous/next edited queries, and undo/redo on query edition.
   class PreviousNextStack<T> : IPreviousNextStack<T> where T : class {

      public PreviousNextStack(IEnumerable<T> items, T defaultItem, Func<T,T, bool> itemsEqualProc) {
         Debug.Assert(items != null);
         Debug.Assert(defaultItem != null);
         Debug.Assert(itemsEqualProc != null);
         m_Previous = new Stack<T>(items);
         m_Next = new Stack<T>();
         m_Current = defaultItem;
         m_ItemsEqualProc = itemsEqualProc;
      }

      private readonly Stack<T> m_Previous;
      private readonly Stack<T> m_Next;
      private readonly Func<T, T, bool> m_ItemsEqualProc;

      private T m_Current;
      T IPreviousNextStack<T>.Current { get { return m_Current; } }

      T IPreviousNextStack<T>.GetPrevious() {
         return StackOp(m_Previous, m_Next);
      }

      T IPreviousNextStack<T>.GetNext() {
         return StackOp(m_Next, m_Previous);
      }

      private T StackOp(Stack<T> stackA, Stack<T> stackB) {
         Debug.Assert(stackA != null);
         Debug.Assert(stackB != null);
         if (stackA.Count == 0) { return m_Current; }
         stackB.Push(m_Current);
         m_Current = stackA.Pop();
         Debug.Assert(m_Current != null);
         return m_Current;
      }

      void IPreviousNextStack<T>.Append(T item) {
         Debug.Assert(item != null);
         if (m_ItemsEqualProc(item, m_Current)) { return; }
         m_Next.Clear();
         m_Previous.Push(m_Current);
         m_Current = item;
      }

      void IPreviousNextStack<T>.AppendButDontClearNext(T item) {
         Debug.Assert(item != null);
         if (m_ItemsEqualProc(item, m_Current)) { return; }
         m_Previous.Push(m_Current);
         m_Current = item;
      }

      void IPreviousNextStack<T>.Clear(T defaultItem){
         Debug.Assert(defaultItem != null);
         m_Current = defaultItem;
         m_Previous.Clear();
         m_Next.Clear();
      }

   }
}
