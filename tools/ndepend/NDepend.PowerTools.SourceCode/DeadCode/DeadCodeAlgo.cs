


using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.Helpers;

namespace NDepend.PowerTools.DeadCode {
   static class DeadCodeAlgo {
      internal static ICodeMetric<IType, ushort> FindDeadTypes(ICodeBase codeBase, Func<IMember, bool> funcHasAttribute) {
         Debug.Assert(codeBase != null);
         Debug.Assert(funcHasAttribute != null);
         // Filter function for types that should'nt be considered as dead
         var canTypeBeConsideredAsDead = new Func<IType, bool>(
            t => !t.IsPublic && //   Public types might be used by client applications of your assemblies.
                 t.Name != "Program" &&
                 !funcHasAttribute(t) &&

                 // Exclude static types that define only const fields
                 // because they cannot be seen as used in IL code.
                 !(t.IsStatic && t.NbMethods == 0 && !t.Fields.Where(f => !f.IsLiteral).Any()));

         // Select types unused
         var typesUnused =
            from t in codeBase.Application.Types where
            t.NbTypesUsingMe == 0 && canTypeBeConsideredAsDead(t)
            select t;

         // Dead types = types used only by unused types (recursive)
         var deadTypesMetric = typesUnused.FillIterative(
         types => from t in codeBase.Application.Types.UsedByAny(types).Except(types)
                  where canTypeBeConsideredAsDead(t) &&
                        t.TypesUsingMe.Intersect(types).Count() == t.NbTypesUsingMe
                  select t);
         return deadTypesMetric;
      }

      internal static ICodeMetric<IMethod, ushort> FindDeadMethods(ICodeBase codeBase, Func<IMember, bool> funcHasAttribute) {
         Debug.Assert(codeBase != null);
         Debug.Assert(funcHasAttribute != null);
         // Filter function for methods that should'nt be considered as dead
         var canMethodBeConsideredAsDead = new Func<IMethod, bool>(
             m => !m.IsPublic &&                // Public methods might be used by client applications of your assemblies.
                  !m.IsEntryPoint &&            // Main() method is not used by-design.
                  !m.IsExplicitInterfaceImpl && // The IL code never explicitely calls explicit interface methods implementation.
                  !m.IsClassConstructor &&      // The IL code never explicitely calls class constructors.
                  !m.IsFinalizer &&             // The IL code never explicitely calls finalizers.
                  !m.IsVirtual &&               // Only check for non virtual method that are not seen as used in IL.
                  !(m.IsConstructor &&          // Don't take account of protected ctor that might be call by a derived ctors.
                    m.IsProtected) &&
                  !m.IsEventAdder &&            // The IL code never explicitely calls events adder/remover.
                  !m.IsEventRemover &&
                  !funcHasAttribute(m) &&
                  !m.FullName.Contains(".Resources."));

         // Get methods unused
         var methodsUnused =
            from m in codeBase.Application.Methods where
            m.NbMethodsCallingMe == 0 &&
            canMethodBeConsideredAsDead(m)
            select m;

         // Dead methods = methods used only by unused methods (recursive)
         var deadMethodsMetric = methodsUnused.FillIterative(
            methods => {
               // Use a hashet to make Intersect calls much faster!
               var hashset = methods.ToHashSet();
               return from m in codeBase.Application.Methods.UsedByAny(methods).Except(methods)
                      where canMethodBeConsideredAsDead(m) &&
                         // Select methods called only by methods already considered as dead
                          hashset.Intersect(m.MethodsCallingMe).Count() == m.NbMethodsCallingMe
                      select m;
            }
         );
         return deadMethodsMetric;
      }

      internal static IEnumerable<IField> FindDeadFields(ICodeBase codeBase, Func<IMember, bool> funcHasAttribute) {
         Debug.Assert(codeBase != null);
         Debug.Assert(funcHasAttribute != null);
         return from f in codeBase.Application.Fields where
            f.NbMethodsUsingMe == 0 &&
            !f.IsPublic &&     // Although not recommended, public fields might be used by client applications of your assemblies.
            !f.IsLiteral &&    // The IL code never explicitely uses literal fields.
            !f.IsEnumValue &&  // The IL code never explicitely uses enumeration value.
            f.Name != "value__" && // Field named 'value__' are relative to enumerations and the IL code never explicitely uses them.
            !funcHasAttribute(f)
                select f;
      }
   }
}
