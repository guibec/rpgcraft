using System.Collections.Generic;
using NUnit.Framework;

namespace NDepend.PowerTools.AppWords {

#if TEST
   [TestFixture]
   public class Test_WordExtracter {
      [TestCase("m_AA_BB", new[] { "Aa", "Bb" })]
      [TestCase("MClr_BBBb", new[] { "Clr", "Bb", "Bb" })]
      [TestCase("CLRTypes", new[] { "Clr", "Type" })]
      [TestCase("CLRLENGTH", new[] { "Clrlength" })]
      [TestCase("X509Certificate", new[] { "X509", "Certificate" })]
      [TestCase("X509Certificates", new[] { "X509", "Certificate" })]
      [TestCase("X509Certificate2", new[] { "X509", "Certificate" })]
      [TestCase("CheckSignature", new[] { /*"Check",*/ "Signature" })]
      [TestCase("AssembliesSignature", new[] { "Assembly", "Signature" })]
      [TestCase("PresentationUI", new[] { "Presentation", /*"Ui"*/ })]
      [TestCase("System.Activities", new[] { "System", "Activity" })]
      [TestCase("System.DirectoryServices.AccountManagement", new[] { "System", "Directory", "Service", "Account", "Management" })]
      [TestCase("System.Windows.Baml2006", new[] { "System", "Window", "Baml", "2006" })]
      [TestCase("NativeMethods+SECURITY_INFORMATION", new[] { "Native", /*"Method",*/ "Security", /*"Information"*/ })]
      [TestCase("Correlation+<>c__DisplayClass6", new[] { "Correlation", "Display", /*"Class"*/ })]
      [TestCase("@hello", new[] { "Hello" })]
      [TestCase(".ctor", new string[0])]
      [TestCase(".cctor", new string[0])]
      [TestCase("get_Type", new[] { "Type" })]
      public void TestMethod1(string identifier, string[] wordsResult) {
         var wordsExtracted = new List<string>(WordExtracter.ExtractWordsFromIdentifier(identifier));
         Assert.IsTrue(wordsExtracted.Count == wordsResult.Length);
         for (var i = 0; i < wordsResult.Length; i++) {
            Assert.IsTrue(wordsExtracted[i] == wordsResult[i]);
         }
      }
   }
#endif
}
