using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.Helpers;


namespace NDepend.PowerTools.SearchForDuplicateCode {


   //
   // SuspectSetsWith2CallersComputer is an algorithm that found pairs of methods that call N or more same methods/fields.
   //
   static class SuspectSetsWith2CallersComputer {

      private static bool IsTooCommonMethod(IMethod method) {
         Debug.Assert(method != null);
         return method.ParentNamespace.Name.EqualsAny("System.Collections", "System.Collections.Generic", "System.Linq", "System.Xml", "System.Diagnostics") ||
                method.ParentType.Name.EqualsAny("Object", "IDisposable");
      }



      internal static List<SuspectSet> Go(ICodeBase codeBase, int N) {
         Debug.Assert(codeBase != null);

         var suspectSetsCallerPair = new List<SuspectSet>();

         // Some methods are oo common, they could provoque a bias in result, hence we don't take account of them.
         var methodsCalledsTooCommon = codeBase.Methods.Where(IsTooCommonMethod);

         // Exclude some generated methods, that typically contains duplicated or very similar code.
         var methodsCallersExcluded = GetExcludedMethod(codeBase);

         // This hashset is used to keep track of methods called already treated.
         var calledsAlreadyTreated = new HashSet<IMethod>(methodsCalledsTooCommon);

         // This hashset is used to keep track of pairs of callers already treated.
         var pairsOfCallersAlreadyTreated = new HashSet<PairOfMethodCallers>();

         //
         // for each methods called, try to find pairs amongst its callers calling at least N same methods!
         //
         foreach (var called in codeBase.Methods.Except(methodsCalledsTooCommon)) {
            var callers = called.MethodsCallingMe;
            Debug.Assert(callers != null);
            if (callers.Count() < 2) { continue; }  // At least 2 callers to get an intersection

            
            // Compute list of callers and the methods they call (except methods called already treated)
            var callersSuspect = (from caller in callers
                               where !methodsCallersExcluded.Contains(caller)
                               let methodsCalled = caller.MethodsCalled
                               where methodsCalled.Count() >= N
                               // Need to call ToArray() to avoid differed query execution. Else calledsAlreadyTreated being modified later could make differed query execution problems!!
                               let methodsCalledExceptThoseAlreadyTreated = methodsCalled.Except(calledsAlreadyTreated).ToArray()
                               let fieldsRead = caller.FieldsReadButNotAssigned.ToArray()
                               let fieldsAssigned = caller.FieldsAssigned.ToArray()
                               // Condition for a caller to become suspect: At least N methods called, fields read, fields assigned
                               where methodsCalledExceptThoseAlreadyTreated.Length + fieldsRead.Count() + fieldsAssigned.Count() >= N
                               select new CallerAndMembersUsed(caller, methodsCalledExceptThoseAlreadyTreated, fieldsRead, fieldsAssigned)).ToArray();

            // Now we can add called to calledsAlreadyTreated!
            calledsAlreadyTreated.Add(called);

            // Needs at least 2 callers methods to get one or several suspect sets with 2 callers.
            if (callersSuspect.Length < 2) { continue; }  

            //
            // Compute pair of callers calling at least N same methods!
            //
            var suspectSetsWith2Callers = FindCallersCallingAtLeastNSameMethods(callersSuspect, pairsOfCallersAlreadyTreated, N);

            suspectSetsCallerPair.AddRange(suspectSetsWith2Callers);
         }  // end foreach method called


         return suspectSetsCallerPair;
      }

      private static HashSet<IMethod> GetExcludedMethod(ICodeBase codeBase) {
         Debug.Assert(codeBase != null);

         // method DeemAsExcludedMethodFromFileName + eventually
         // methods tagged with generatedCodeAttributeType + types tagged with generatedCodeAttributeType
         var methodsExcluded = codeBase.Application.Methods.Where(DeemAsExcludedMethodFromFileName);

         var generatedCodeAttributeType = codeBase.ThirdParty.Types.WithFullName("System.CodeDom.Compiler.GeneratedCodeAttribute").SingleOrDefault();
         if (generatedCodeAttributeType != null) {
            methodsExcluded = methodsExcluded.Concat(codeBase.Application.Methods.Where(m => m.HasAttribute(generatedCodeAttributeType)));
            methodsExcluded = methodsExcluded.Concat(codeBase.Application.Types.Where(t => t.HasAttribute(generatedCodeAttributeType)).ChildMethods());
         }

         return new HashSet<IMethod>(methodsExcluded);
      }


      private static bool DeemAsExcludedMethodFromFileName(IMethod method) {
         Debug.Assert(method != null);
         if (!method.SourceFileDeclAvailable) { return false; }
         var sourceFilePathLower = method.SourceDecls.Single().SourceFile.FilePathString.ToLower();

         if (sourceFilePathLower.EndsWithAny(
         ".g.cs",        // Popular pattern to name generated files.
         ".g.vb",
         ".xaml",        // notmycode WPF xaml code
         ".designer.cs", // notmycode C# and VB.NET Windows Forms designer code ...
         ".designer.vb"  // ... and Resources designer code
         )) { return true; }

         return sourceFilePathLower.Contains("generated");

      }


      private static IEnumerable<SuspectSet> FindCallersCallingAtLeastNSameMethods(CallerAndMembersUsed[] callersSuspect, HashSet<PairOfMethodCallers> pairsOfCallersAlreadyTreated, int N) {
         Debug.Assert(callersSuspect != null);
         Debug.Assert(callersSuspect.Length >= 2);
         Debug.Assert(pairsOfCallersAlreadyTreated != null);

         var suspectSetsWith2Callers =new List<SuspectSet>();


         MembersUsedIntersectionComputer<CallerAndMembersUsed>.Go(callersSuspect, N,
            // This lambda is called to know if a pair of CallerAndMembersUsed should be treated or not.
            (callerI, callerJ) => {
               var pair = new PairOfMethodCallers(callerI.MethodCaller, callerJ.MethodCaller);
               // Avoid treating a pair of method callers already reated!
               if (pairsOfCallersAlreadyTreated.Contains(pair)) { return false; }
               pairsOfCallersAlreadyTreated.Add(pair);
               return true;
            },

            // This lambda is called when a merge should occurs.
            (callerI, callerJ, methodCalledsIntersection, fieldsReadIntersection, fieldsAssignedIntersection) => {
               // Merge suspectSetI and suspectSetJ
               var suspectSet = new SuspectSet(new List<IMethod> { callerI.MethodCaller, callerJ.MethodCaller }, methodCalledsIntersection, fieldsReadIntersection, fieldsAssignedIntersection);
               suspectSetsWith2Callers.Add(suspectSet);
               return false; // <-- This provoques continue current loop
            });


         return suspectSetsWith2Callers;
      }



      //
      // 2 private classes used by the algorithm.
      //

      // PairOfMethodCallers is useful to hold a reference on a pair of methods, and compute a hash from the method pair.
      private class PairOfMethodCallers {
         internal PairOfMethodCallers(IMethod callerA, IMethod callerB) {
            m_CallerA = callerA;
            m_CallerB = callerB;
         }

         private readonly IMethod m_CallerA, m_CallerB;

         public override int GetHashCode() {
            return m_CallerA.UniqueID * m_CallerB.UniqueID;
         }

         public override bool Equals(object obj) {
            var other = obj as PairOfMethodCallers;
            Debug.Assert(other != null);
            if (other.m_CallerA == this.m_CallerA && other.m_CallerB == this.m_CallerB) { return true; }
            if (other.m_CallerA == this.m_CallerB && other.m_CallerB == this.m_CallerA) { return true; }
            return false;
         }
      }


      // Holds a caller and its members called.
      class CallerAndMembersUsed : MembersUsedBase {
         internal CallerAndMembersUsed(IMethod methodCaller, IList<IMethod> methodsCalled, IList<IField> fieldsRead, IList<IField> fieldsAssigned)
            : base(methodsCalled, fieldsRead, fieldsAssigned) {
               Debug.Assert(methodCaller != null);
               m_MethodCaller = methodCaller;
         }

         internal IMethod MethodCaller { get { return m_MethodCaller; } }
         private readonly IMethod m_MethodCaller;
      }

   }
}
