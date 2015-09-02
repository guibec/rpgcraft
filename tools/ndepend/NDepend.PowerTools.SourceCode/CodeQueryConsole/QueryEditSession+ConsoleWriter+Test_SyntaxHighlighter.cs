

#if TEST
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;



namespace NDepend.PowerTools.CodeQueryConsole {


   partial class QueryEditSession {

      partial class ConsoleWriter {

         [TestFixture]
         public class Test_SyntaxHighlighter {

            [Test]
            public void Test_OneLine1() {
               OneLineTest("let /**/ //hello", 
                  new Highlight(0, 3, KEYWORD_FORE_COLOR, BACK_COLOR),
                  new Highlight(3, 1, FORE_COLOR, BACK_COLOR),
                  new Highlight(4, 4, COMMENT_FORE_COLOR, BACK_COLOR),
                  new Highlight(8, 1, FORE_COLOR, BACK_COLOR),
                  new Highlight(9, 7, COMMENT_FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine2() {
               OneLineTest("let/**///hello",
                  new Highlight(0, 3, KEYWORD_FORE_COLOR, BACK_COLOR),
                  new Highlight(3, 4, COMMENT_FORE_COLOR, BACK_COLOR),
                  new Highlight(7, 7, COMMENT_FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine_WithNoHighlight() {
               OneLineTest("hello",
                  new Highlight(0, 5, FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine_RemoveOverlappingHighlights() {
               OneLineTest("/*let*/",
                  new Highlight(0, 7, COMMENT_FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine_HighlightAfterComment() {
               OneLineTest("/**/hello",
                  new Highlight(0, 4, COMMENT_FORE_COLOR, BACK_COLOR),
                  new Highlight(4, 5, FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine_QueryNaming() {
               OneLineTest("/*<Name>name</Name>*/",
                  new Highlight(0, 8, COMMENT_FORE_COLOR, BACK_COLOR),
                  new Highlight(8, 4, QUERY_NAME_FORE_COLOR, QUERY_NAME_BACK_COLOR),
                  new Highlight(12, 9, COMMENT_FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine_QueryNaming_NoEndTag() {
               OneLineTest("/*<Name>name<Name>*/",
                  new Highlight(0, 20, COMMENT_FORE_COLOR, BACK_COLOR));
            }

            [Test]
            public void Test_OneLine_QueryNaming_EndTagBeforeStartTag() {
               OneLineTest("/*</Name>name<Name>*/",
                  new Highlight(0, 21, COMMENT_FORE_COLOR, BACK_COLOR));
            }



            private static void OneLineTest(string line, params Highlight[] highlights) {
               var result = SyntaxHighlighter.Go(new[] { line }.ToList()).First();
               HighlihtsCollectionEquality(result, highlights);
            }


            [Test]
            public void Test_MultilineCommentContinuedOnNextLine() {
               var results = SyntaxHighlighter.Go(new[] { "/*hello" , "hello*/hello" }.ToList());

               HighlihtsCollectionEquality(results[0],
                                           new Highlight(0, 7, COMMENT_FORE_COLOR, BACK_COLOR));
               HighlihtsCollectionEquality(results[1],
                                           new Highlight(0, 7, COMMENT_FORE_COLOR, BACK_COLOR),
                                           new Highlight(7, 5, FORE_COLOR, BACK_COLOR));
            }






            private static void HighlihtsCollectionEquality(IList<Highlight> highlightsA, params Highlight[] highlightsB) {
               Assert.IsTrue(highlightsA.Count == highlightsB.Length);
               for (var i = 0; i < highlightsA.Count; i++) {
                  var a = highlightsA[i];
                  var b = highlightsB[i];
                  Assert.IsTrue(a.StartPosition == b.StartPosition);
                  Assert.IsTrue(a.Length == b.Length);
                  Assert.IsTrue(a.ForegroundColor == b.ForegroundColor);
                  Assert.IsTrue(a.BackgroundColor == b.BackgroundColor);
               }
               
            }
            
         }
      }
   }
}

#endif
