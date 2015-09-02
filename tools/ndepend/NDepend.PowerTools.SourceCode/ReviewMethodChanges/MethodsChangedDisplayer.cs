

using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.CodeModel;
using NDepend.PowerTools.SharedUtils;

namespace NDepend.PowerTools.ReviewMethodChanges {

   class MethodsChangedDisplayer : MembersDisplayer {
      internal MethodsChangedDisplayer(IList<IMember> members, ICompareContext compareContext) : base(members) {
         Debug.Assert(members != null);
         Debug.Assert(compareContext != null);
         m_CompareContext = compareContext;
      }

      private readonly ICompareContext m_CompareContext;

      protected override void EnterCodeElement(ICodeElement codeElement) {
         Debug.Assert(codeElement != null);
         var method = codeElement as IMethod;
         Debug.Assert(method != null);
         Debug.Assert(m_CompareContext.CodeWasChanged(method));
         var olderMethod = m_CompareContext.OlderVersion(method);
         Debug.Assert(olderMethod != null);
         ConsoleUtils.TryCompareSource(olderMethod.SourceDecls.FirstOrDefault(), 
                                       method.SourceDecls.FirstOrDefault(), 
                                       "Source file decl not available for one of the method");
      }
   }

}