using System.Collections.Generic;
using System.Diagnostics;
using NDepend.CodeModel;

namespace NDepend.PowerTools.SearchForDuplicateCode {

   //
   // The SuspectSet class represents a set of at least 2 callers methods, all calling at least N calleds methods.
   //

   abstract class MembersUsedBase {
      protected MembersUsedBase(IList<IMethod> methodsCalled, IList<IField> fieldsRead, IList<IField> fieldsAssigned) {
         Debug.Assert(methodsCalled != null);
         Debug.Assert(fieldsRead != null);
         Debug.Assert(fieldsAssigned != null);
         Debug.Assert(methodsCalled.Count + fieldsRead.Count + fieldsAssigned.Count >= SearchForDuplicateCodePowerTool.N);
         m_MethodsCalled = methodsCalled;
         m_FieldsRead = fieldsRead;
         m_FieldsAssigned = fieldsAssigned;
      }

      internal IList<IMethod> MethodsCalled { get { return m_MethodsCalled; } }
      private readonly IList<IMethod> m_MethodsCalled;

      internal IList<IField> FieldsRead { get { return m_FieldsRead; } }
      private readonly IList<IField> m_FieldsRead;

      internal IList<IField> FieldsAssigned { get { return m_FieldsAssigned; } }
      private readonly IList<IField> m_FieldsAssigned;

   }


   class SuspectSet : MembersUsedBase {
      internal SuspectSet(IList<IMethod> methodsCaller, IList<IMethod> methodsCalled, IList<IField> fieldsRead, IList<IField> fieldsAssigned) 
           : base(methodsCalled, fieldsRead, fieldsAssigned) {
         Debug.Assert(methodsCaller.Count >= 2);
         m_MethodsCaller = methodsCaller;
      }

      internal IList<IMethod> MethodsCaller { get { return m_MethodsCaller; } }
      private readonly IList<IMethod> m_MethodsCaller;
   }
}
