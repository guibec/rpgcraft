
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace NDepend.PowerTools.SearchForDuplicateCode {
   static class SuspectSetsSorter {

      // The sort happens on the inputed list itself.
      internal static void Go(List<SuspectSet> suspectSets) {
         Debug.Assert(suspectSets != null);

         // Compute a weight for each suspectSets, and sort them according to this weight.
         var suspectnesses = new Dictionary<SuspectSet, int>();

         foreach (var suspectSet in suspectSets) {
            var nbApplicationMethodsCalleds = suspectSet.MethodsCalled.Count(m => !m.IsThirdParty);
            var nbApplicationFieldsRead = suspectSet.FieldsRead.Count(f => !f.IsThirdParty);
            var nbFieldsAssigned = suspectSet.FieldsAssigned.Count;

            // Compute weight of suspectSet, its suspectness
            // Notice fieldsAssigned counts double, and third-party methods/fields read are not taken account.
            var suspectness = nbApplicationMethodsCalleds + nbApplicationFieldsRead + 2 * nbFieldsAssigned;

            suspectnesses.Add(suspectSet, suspectness);
         }


         suspectSets.Sort((s1, s2) => {
            var s1Suspectness = suspectnesses[s1];
            var s2Suspectness = suspectnesses[s2];
            return s1Suspectness < s2Suspectness ? 1 : s1Suspectness > s2Suspectness ? -1 :
                   s1.MethodsCaller.Count < s2.MethodsCaller.Count ? 1 : s1.MethodsCaller.Count > s2.MethodsCaller.Count ? -1 : 0;
         });

      }

   }
}
