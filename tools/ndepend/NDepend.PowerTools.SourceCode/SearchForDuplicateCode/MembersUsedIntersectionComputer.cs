using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.Helpers;

namespace NDepend.PowerTools.SearchForDuplicateCode {

   // This Fnc is not defined in .NET 3.5!
   internal delegate TResult Func<in T0, in T1, in T2, in T3, in T4, out TResult>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4);

   static class MembersUsedIntersectionComputer<T> where T : MembersUsedBase {

      internal static void Go(T[] items, int N, Func<T, T, bool> funcShouldTreatPair, Func<T, T, IList<IMethod>, IList<IField>, IList<IField>, bool> funcMerge) {
         Debug.Assert(items != null);
         Debug.Assert(funcShouldTreatPair != null);
         Debug.Assert(funcMerge != null);

         // Optimization: bufferize the array length
         var listLength = items.Length;

         for (var i = 0; i < listLength; i++) {
         var itemI = items[i];

            // Optimization: use a hashset to compute quickly intersections!
            var hahsetMethodsCalled = itemI.MethodsCalled.ToHashSet();
            var hahsetFieldsRead = itemI.FieldsRead.ToHashSet();
            var hahsetFieldsAssigned = itemI.FieldsAssigned.ToHashSet();

            for (var j = i + 1; j < listLength; j++) {
               var itemJ = items[j];

               // SHould we treat this pair of item?
               if (!funcShouldTreatPair(itemI, itemJ)) { continue; }

               var methodCalledsIntersection = hahsetMethodsCalled.Intersect(itemJ.MethodsCalled).ToArray();
               var fieldsReadIntersection = hahsetFieldsRead.Intersect(itemJ.FieldsRead).ToArray();
               var fieldsAssignedIntersection = hahsetFieldsAssigned.Intersect(itemJ.FieldsAssigned).ToArray();

               // The 2 suspects set are merged if and only if intersection contains at least N members.
               if (methodCalledsIntersection.Length + fieldsReadIntersection.Length + fieldsAssignedIntersection.Length < N) { continue; }

               // Call the merge function and eventually break or continue loop
               if (funcMerge(itemI, itemJ, methodCalledsIntersection, fieldsReadIntersection, fieldsAssignedIntersection)) { break; }
            }
         }
      }


   }
}
