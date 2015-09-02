using System.Diagnostics;
using NDepend.CodeModel;

namespace NDepend.PowerTools.AppWords {

   class IdentifierWeight {
      internal IdentifierWeight(string identifier, float? weight) { Identifier = identifier; Weight = weight; }
      internal string Identifier { get; private set; }
      internal float? Weight { get; private set; }
   }


   internal static class IdentifierWeightComputer {

      // If LoC bot available, us #Il instructions divided by 7, which reflect approximatively the LoC.
      const int RATIO_LOC_IL_INSTRUCTIONS = 7;

      // Factors to compute weight of identifiers
      const int TYPE_WEIGHT = 10;
      const int TYPE_RANK_MULTIPLIER_WEIGHT = 10;
      const int METHOD_RANK_MULTIPLIER_WEIGHT = 10;
      const int FIELD_WEIGHT = 20;

      internal static IdentifierWeight GetWeigth(IAssembly assembly) {
         Debug.Assert(assembly != null);
         var locWeight = assembly.NbLinesOfCode ?? assembly.NbILInstructions / RATIO_LOC_IL_INSTRUCTIONS;
         var nbTypesWeight = assembly.NbTypes * TYPE_WEIGHT;
         return new IdentifierWeight(assembly.Name, locWeight + nbTypesWeight);
      }

      internal static IdentifierWeight GetWeigth(INamespace @namespace) {
         Debug.Assert(@namespace != null);
         var locWeight = @namespace.NbLinesOfCode ?? @namespace.NbILInstructions / RATIO_LOC_IL_INSTRUCTIONS;
         var nbTypesWeight = @namespace.NbTypes * TYPE_WEIGHT;
         return new IdentifierWeight(@namespace.Name, locWeight + nbTypesWeight);
      }

      internal static IdentifierWeight GetWeigth(IType type) {
         Debug.Assert(type != null);
         var locWeight = type.NbLinesOfCode ?? type.NbILInstructions / RATIO_LOC_IL_INSTRUCTIONS;
         var typeRank = type.Rank * TYPE_RANK_MULTIPLIER_WEIGHT;
         // Remove generic decl
         var name = type.Name;
         var index = name.IndexOf('<');
         if (index > 0) { name = name.Substring(0, index); }
         return new IdentifierWeight(name, locWeight + typeRank);
      }

      internal static IdentifierWeight GetWeigth(IMethod method) {
         Debug.Assert(method != null);
         var locWeight = method.NbLinesOfCode ?? method.NbILInstructions / RATIO_LOC_IL_INSTRUCTIONS;
         var typeRank = method.Rank * METHOD_RANK_MULTIPLIER_WEIGHT;
         // Remove generic decl and signature
         var name = method.Name;
         var index = name.IndexOf('(');
         if (index > 0) { name = name.Substring(0, index); }
         index = name.IndexOf('<');
         if (index > 0) { name = name.Substring(0, index); }
         return new IdentifierWeight(name, locWeight + typeRank);
      }

      internal static IdentifierWeight GetWeigth(IField field) {
         Debug.Assert(field != null);
         return new IdentifierWeight(field.Name, FIELD_WEIGHT);
      }
   }
}
