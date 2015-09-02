using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.Helpers;

namespace NDepend.PowerTools.SearchForDuplicateCode {

   //
   // SuspectSetsMerger is an algorithm that transforms a collection of suspectSets with 2 callers methods
   // into a collection of suspectSets with 2 or more callers.
   //
   static class SuspectSetsMerger {
      internal static List<SuspectSet> Go(List<SuspectSet> suspectSetsWith2Callers, int N) {
         Debug.Assert(suspectSetsWith2Callers != null);
         //
         // GetCollectionOfSuspectSetsSharingACommonCaller() groups suspectSets with 2 callers methods, that share a common caller method.
         //
         IEnumerable<IList<SuspectSet>> collectionOfSuspectSetsSharingACommonCaller = GetCollectionOfSuspectSetsSharingACommonCaller(suspectSetsWith2Callers);

         //
         // Merge suspectSets
         //
         var suspectSetsFinal = new List<SuspectSet>();
         foreach (var suspectSetsSharingACommonCaller in collectionOfSuspectSetsSharingACommonCaller) {
            var suspectSetsMerged = MergeSuspectSetsSharingACommonCaller(suspectSetsSharingACommonCaller, N);
            suspectSetsFinal.AddRange(suspectSetsMerged);
         }
         return suspectSetsFinal;
      }


      //
      // Inputed suspectSets that shares a common caller, all have 2 callers.
      //
      private static IEnumerable<SuspectSet> MergeSuspectSetsSharingACommonCaller(IList<SuspectSet> suspectSetsSharingACommonCaller, int N) {
         Debug.Assert(suspectSetsSharingACommonCaller != null);
         if (suspectSetsSharingACommonCaller.Count == 1) {
            return suspectSetsSharingACommonCaller;
         }

         var suspectSetsMergedFinal = new List<SuspectSet>();

         // suspectSetsMergedCurrent  => i.e merged during the current iteration
         List<SuspectSet> suspectSetsMergedCurrent;
         // suspectSetsMergedPrevious => i.e merged during previous iteration
         SuspectSet[] suspectSetsMergedPrevious = suspectSetsSharingACommonCaller.ToArray();

         //
         // Iteratively merge suspectSets when possible, until no more suspectSets can be merged.
         //
         do {
            suspectSetsMergedCurrent = new List<SuspectSet>();

            // Optimization: use a hashset to have a constant time remove.
            var suspectSetsNotMergeableAnymore = suspectSetsMergedPrevious.ToHashSet();


            //
            // Try to merge suspectSets merged during the previous iteration.
            //
            MembersUsedIntersectionComputer<SuspectSet>.Go(suspectSetsMergedPrevious, N ,

               // Always see if a pair of SuspectSet can be merged
               (callerI, callerJ) => true,  
                 
               // This lambda is called when a merge should occurs.
               (suspectSetI, suspectSetJ, methodCalledsIntersection, fieldsReadIntersection, fieldsAssignedIntersection) => {
                  // Merge suspectSetI and suspectSetJ
                  var callersUnion = suspectSetI.MethodsCaller.Union(suspectSetJ.MethodsCaller).ToArray();
                  suspectSetsMergedCurrent.Add(new SuspectSet(callersUnion, methodCalledsIntersection, fieldsReadIntersection, fieldsAssignedIntersection));
                  suspectSetsNotMergeableAnymore.Remove(suspectSetI);
                  suspectSetsNotMergeableAnymore.Remove(suspectSetJ);
                  return true; // <-- This provoques break current loop
               });

            suspectSetsMergedPrevious = suspectSetsMergedCurrent.ToArray();
            suspectSetsMergedFinal.AddRange(suspectSetsNotMergeableAnymore);

         } while (suspectSetsMergedCurrent.Count > 0);
         return suspectSetsMergedFinal;
      }



      //
      // GetCollectionOfSuspectSetsSharingACommonCaller() groups suspectSets with 2 callers methods, that share a common caller method.
      // To avoid that a set with 2 callers methods ends up in 2 groups, we define a total order on methods.
      //
      private static IEnumerable<IList<SuspectSet>> GetCollectionOfSuspectSetsSharingACommonCaller(IEnumerable<SuspectSet> suspectSetsWith2Callers) {
         Debug.Assert(suspectSetsWith2Callers != null);
         var dico = new Dictionary<IMethod, IList<SuspectSet>>();
         foreach (var suspectSet in suspectSetsWith2Callers) {
            var callers = suspectSet.MethodsCaller;
            Debug.Assert(callers.Count == 2);

            var callerA = callers[0];
            var callerB = callers[1];
            // IMethod.UniqueID defines a total order on IMethod of a same code base.
            IMethod caller = callerA.UniqueID > callerB.UniqueID ? callerA : callerB;

            IList<SuspectSet> list;
            if (dico.TryGetValue(caller, out list)) {
               list.Add(suspectSet);
               continue;
            }
            list = new List<SuspectSet> { suspectSet };
            dico.Add(caller, list);
         }
         return dico.Values;
      }
   }
}
