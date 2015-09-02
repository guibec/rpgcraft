
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using NDepend.Attributes;

namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {

      partial class ConsoleWriter {

         // This non-trivial class is 100% unit-tested by Test_SyntaxHighlighter
         [FullCovered]
         static class SyntaxHighlighter {

            internal static List<Highlight>[] Go(List<string> lines) {
               Debug.Assert(lines != null);
               var nbLines = lines.Count;
               Debug.Assert(nbLines > 0);
               var arrayResult = new List<Highlight>[nbLines];

               bool beginInComment = false;
               for (var i = 0; i < nbLines; i++) {
                  var line = lines[i];
                  var highlights = new List<Highlight>();
                  FillHighlightsKeywords(line, highlights);
                  beginInComment = FillHighlightsComments(0, line, highlights, beginInComment);
                  RemoveOverlappingHighlights(highlights);
                  ExtractQueryNameHighlights(line, highlights);
                  // would be nice to also highlights string literals in dark red!
                  FillHighlightsInBetween(line, highlights);
                  arrayResult[i] = highlights;
               }
               return arrayResult;
            }






            #region Keywords highlight
            //-----------------------------------------------------------------------------------
            //
            //   Keywords highlight
            //
            //-----------------------------------------------------------------------------------
            private static void FillHighlightsKeywords(string line, List<Highlight> highlights) {
               Debug.Assert(line != null);
               Debug.Assert(highlights != null);

               foreach (var keyword in s_Keywords) {
                  var index = line.IndexOf(keyword);
                  while (index >= 0) {
                     if (IsKeyword(line, index, keyword)) {
                        highlights.Add(new Highlight(index, keyword.Length, KEYWORD_FORE_COLOR, BACK_COLOR));
                     }
                     index = line.IndexOf(keyword, index + 1);
                  }
               }
            }
            private static bool IsKeyword(string line, int index, string keyword) {
               Debug.Assert(!string.IsNullOrEmpty(line));
               Debug.Assert(!string.IsNullOrEmpty(keyword));
               Debug.Assert(index >= 0);
               var indexEnd = index + keyword.Length;
               Debug.Assert(indexEnd <= line.Length);
               char? charBefore = (index == 0) ? (char?)null : line[index - 1];
               char? charAfter = (indexEnd >= line.Length) ? (char?)null : line[indexEnd];
               return (charBefore == null || (!char.IsLetter(charBefore.Value) && charBefore.Value != '@')) &&
                      (charAfter == null || (!char.IsLetter(charAfter.Value) && !char.IsDigit(charAfter.Value)));
            }

            private readonly static string[] s_Keywords = new[] {
               // C#  (not all C# keywords can be used in a LINQ expression)
               "null", "new", "as", "is", "default", "true", "false", "ref", "in", "out", "sizeof", "typeof", 
               "object", "byte", "sbyte", "short", "ushort", "int", "uint", "long", "ulong", "float", "double", "decimal", "bool", "char",

               // LINQ / C#
               "warnif", "let", "from", "select", "where", "orderby", "descending", "ascending", 
               "join", "group", "into", "on", "equals",

               // CQL
               "SELECT", "TOP", "FROM", "WHERE", "ORDER", "BY", "DESC", "ASC", "AND", "OR", "WARN", "IF", "OUT", "OF",
               "METHODS", "FIELDS", "TYPES", "NAMESPACES", "ASSEMBLIES",
            };
            #endregion





            #region Comments highlight
            //-----------------------------------------------------------------------------------
            //
            //   Comments highlight
            //
            //-----------------------------------------------------------------------------------

            private const string BEGIN_MULTILINE_COMMENT = "/*";
            private const string END_MULTILINE_COMMENT = "*/";
            private const string SINGLE_LINE_COMMENT = "//";

            private static bool FillHighlightsComments(int index, string line, List<Highlight> highlights, bool beginInComment) {
               Debug.Assert(index >= 0);
               Debug.Assert(index <= line.Length);
               Debug.Assert(highlights != null);
               Debug.Assert(line != null);
               
               if(beginInComment) {
                  return TryEndMultiLineComment(index, line, highlights);
               }

               var indexBeginMultiLinesComment = line.IndexOf(BEGIN_MULTILINE_COMMENT, index);
               var indexBeginSingleLineComment = line.IndexOf(SINGLE_LINE_COMMENT, index);

               if (indexBeginMultiLinesComment == -1 && indexBeginSingleLineComment == -1) {
                  return false;
               }
               if (indexBeginMultiLinesComment == -1 || (indexBeginSingleLineComment != -1 && indexBeginSingleLineComment < indexBeginMultiLinesComment)) {
                  AppendSingleLineComment(indexBeginSingleLineComment, line, highlights);
                  return false;
               }
               Debug.Assert(indexBeginSingleLineComment == -1 || indexBeginMultiLinesComment < indexBeginSingleLineComment);
               return TryEndMultiLineComment(indexBeginMultiLinesComment, line, highlights);
            }

            private static void AppendSingleLineComment(int index, string line, List<Highlight> highlights) {
               Debug.Assert(index >= 0);
               Debug.Assert(index < line.Length);
               Debug.Assert(highlights != null);
               Debug.Assert(line != null);
               Debug.Assert(line.IndexOf(SINGLE_LINE_COMMENT, index) == index);
               highlights.Add(new Highlight(index, line.Length - index, COMMENT_FORE_COLOR, BACK_COLOR));
            }

            private static bool TryEndMultiLineComment(int index, string line, List<Highlight> highlights) {
               Debug.Assert(index >= 0);
               Debug.Assert(index <= line.Length);
               Debug.Assert(highlights != null);
               Debug.Assert(line != null);

               var indexEnd = line.IndexOf(END_MULTILINE_COMMENT, index);
               if (indexEnd == -1) {
                  // The multiline comments continue on next line!
                  highlights.Add(new Highlight(index, line.Length - index, COMMENT_FORE_COLOR, BACK_COLOR));
                  return true;
               }
               indexEnd += END_MULTILINE_COMMENT.Length;
               Debug.Assert(indexEnd > index);
               highlights.Add(new Highlight(index, indexEnd - index, COMMENT_FORE_COLOR, BACK_COLOR));
               return FillHighlightsComments(indexEnd, line, highlights, false);
            }
            #endregion Comments highlight



            // Comments highlights can contains keywords highlights
            private static void RemoveOverlappingHighlights(List<Highlight> highlights) {
               Debug.Assert(highlights != null);
               Sort(highlights);

               var highlightsToRemove = new HashSet<Highlight>();

               var count = highlights.Count;
               for (var i = 0; i < count; i++) {
                  var highlightI = highlights[i];
                  for (var j = i+1; j < count; j++) {
                     var highlightJ = highlights[j];
                     Debug.Assert(highlightI.StartPosition <= highlightJ.StartPosition); // coz sorted
                     if (highlightI.StartPosition + highlightI.Length <= highlightJ.StartPosition) {
                        break; // no need to continue since highlights are sorted
                     }
                     // Here I and J overlap, actually I must contain J.
                     Debug.Assert(highlightI.StartPosition + highlightI.Length >= highlightJ.StartPosition + highlightJ.Length);
                     highlightsToRemove.Add(highlightJ);
                  }
               }
               highlights.RemoveAll(highlightsToRemove.Contains);
            }


            private const string s_BeginQueryNameTag = "<Name>";
            private const string s_EndQueryNameTag = "</Name>";
            private static void ExtractQueryNameHighlights(string line, List<Highlight> highlights) {
               Debug.Assert(line != null);
               Debug.Assert(highlights != null);
               var namingHighlights = new List<Highlight>();
               highlights.RemoveAll(highlight => {
                  if (highlight.ForegroundColor != COMMENT_FORE_COLOR) { return false; }
                  var text = line.Substring(highlight.StartPosition, highlight.Length);
                  var indexBegin = text.IndexOf(s_BeginQueryNameTag);
                  if (indexBegin == -1) { return false; }
                  var indexEnd = text.IndexOf(s_EndQueryNameTag);
                  if (indexEnd == -1) { return false; }
                  var indexAfterBegin = indexBegin + s_BeginQueryNameTag.Length;
                  if (indexEnd <= indexAfterBegin) { return false; }
                  namingHighlights.Add(new Highlight(highlight.StartPosition, indexAfterBegin - highlight.StartPosition, COMMENT_FORE_COLOR, BACK_COLOR));
                  namingHighlights.Add(new Highlight(indexAfterBegin, indexEnd - indexAfterBegin, QUERY_NAME_FORE_COLOR, QUERY_NAME_BACK_COLOR));
                  namingHighlights.Add(new Highlight(indexEnd, highlight.StartPosition + highlight.Length - indexEnd, COMMENT_FORE_COLOR, BACK_COLOR));
                  return true;
               });
               highlights.AddRange(namingHighlights);
            }



            private static void FillHighlightsInBetween(string line, List<Highlight> highlights) {
               Debug.Assert(line != null);
               Debug.Assert(highlights != null);

               // Sort Highlighters
               Sort(highlights);

               // Append first
               var inBetween = new List<Highlight>();
               var firstPos = highlights.Count == 0 ? line.Length : highlights.First().StartPosition;
               inBetween.Add(new Highlight(0, firstPos, FORE_COLOR, BACK_COLOR));

               // Append inBetween inter
               for (var i = 1; i < highlights.Count; i++) {
                  var previous = highlights[i - 1];
                  var current = highlights[i];
                  var previousEndPos = previous.StartPosition + previous.Length;
                  Debug.Assert(current.StartPosition >= previousEndPos);
                  if (current.StartPosition == previousEndPos) { continue; }
                  inBetween.Add(new Highlight(previousEndPos, current.StartPosition - previousEndPos, FORE_COLOR, BACK_COLOR));
               }

               // Append last
               if (highlights.Count > 0) {
                  var last = highlights.Last();
                  var lastEndPos = last.StartPosition + last.Length;
                  if (line.Length > lastEndPos) {
                     inBetween.Add(new Highlight(lastEndPos, line.Length - lastEndPos, FORE_COLOR, BACK_COLOR));
                  }
               }

               // Add range inbetween and Sort again Highlighters
               highlights.AddRange(inBetween);

               // make sure o remove empty highlights
               highlights.RemoveAll(highlight => highlight.Length == 0);
               Sort(highlights);
            }


            private static void Sort(List<Highlight> highlights) {
               Debug.Assert(highlights != null);
               highlights.Sort((a, b) => a.StartPosition < b.StartPosition ? -1 : a.StartPosition > b.StartPosition ? 1 :
                                         a.Length > b.Length               ? -1 : a.Length < b.Length               ? 1 : 0);
            }




         }

      }
   }
}
