

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows.Forms;
using NDepend.DotNet;
using NDepend.Path;
using NDepend.PowerTools.Base;
using NDepend.PowerTools.SharedUtils;
using NDepend.Project;

namespace NDepend.PowerTools.AnalyzeAssembliesInFolder {

   class AnalyzeAssembliesInFolder : IPowerTool {
      public string Name { get { return "Analyze Assemblies in Folder"; } }

      public string[] Description {
         get {
            return new[] {
            "Gather .NET assemblies under the folder specified by the user and analyze them.",
            "User can choose to do a recursive search in the folder."
         };
         }
      }

      public void Run() {

         //
         // Get dir
         //

       
         var folderBrowserDialog = new FolderBrowserDialog {ShowNewFolderButton = false};
         if (folderBrowserDialog.ShowDialog() != DialogResult.OK) { return; }
         var dir = folderBrowserDialog.SelectedPath.ToAbsoluteDirectoryPath();

         //
         // Get recursive
         //
         bool recursive = false;
         Console.BackgroundColor = ConsoleColor.Black;
         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine("Search assemblies recursive under {" + dir.ToString() + "} ?");
         Console.WriteLine("Yes?  y    or No? (another key)");
         var consoleKeyInfo = Console.ReadKey();
         if (consoleKeyInfo.KeyChar == 'y') {
            recursive = true;
         }
         Console.ForegroundColor = ConsoleColor.White;


         //
         // Get assembliesPath
         //
         var dotNetManager = new NDependServicesProvider().DotNetManager;
         var assembliesPath = new List<IAbsoluteFilePath>();
         var cursorTop = Console.CursorTop;
         if (!recursive) {
            ScanDir(dir, assembliesPath, dotNetManager, cursorTop);
         } else {
            ScanDirRecursive(dir, assembliesPath, dotNetManager, Console.CursorTop);
         }
         Console.CursorTop = cursorTop;
         Console.CursorLeft = 0;
         ConsoleUtils.ShowNLinesOnConsole(10, ConsoleColor.Black);
         Console.CursorTop = cursorTop;
         Console.CursorLeft = 0;

         //
         // Get project
         //
         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine(assembliesPath.Count + " assemblies found");
         Console.WriteLine(assembliesPath.Select(path => path.FileNameWithoutExtension).Distinct(StringComparer.InvariantCultureIgnoreCase).Count() + " assemblies with distint names found");
         Console.WriteLine("Create the NDepend temporary project.");
         var projectManager = new NDependServicesProvider().ProjectManager;
         var project = projectManager.CreateTemporaryProject(assembliesPath, TemporaryProjectMode.Temporary);

         //
         // Run analysis
         //
         Console.ForegroundColor = ConsoleColor.Yellow;
         Console.WriteLine("Run Analysis!");
         var analysisResult = ProjectAnalysisUtils.RunAnalysisShowProgressOnConsole(project);

         //
         // Show results
         //
         ProjectAnalysisUtils.ShowConsoleAnalysisResult(analysisResult.CodeBase);
      }

      private static void ScanDirRecursive(IAbsoluteDirectoryPath dir, List<IAbsoluteFilePath> assembliesPath, IDotNetManager dotNetManager, int cursorTop) {
         Debug.Assert(dir != null);
         Debug.Assert(dir.Exists);
         Debug.Assert(assembliesPath != null);
         Debug.Assert(dotNetManager != null);
         ScanDir(dir, assembliesPath, dotNetManager, cursorTop);
         foreach (var dirChild in dir.ChildrenDirectoriesPath) {
            ScanDirRecursive(dirChild, assembliesPath, dotNetManager,cursorTop);
         }
      }


      private static void ScanDir(IAbsoluteDirectoryPath dir, List<IAbsoluteFilePath> assembliesPath, IDotNetManager dotNetManager, int cursorTop) {
         Debug.Assert(dir != null);
         Debug.Assert(dir.Exists);
         Debug.Assert(assembliesPath != null);
         Debug.Assert(dotNetManager != null);

         Console.CursorTop = cursorTop;
         Console.CursorLeft = 0;
         Console.WriteLine(assembliesPath.Count + " assemblies found");
         Console.Write("Scanning {" + dir.ToString() + "} ");
         foreach (var filePath in dir.ChildrenFilesPath) {
            if (!dotNetManager.IsAssembly(filePath)) { continue; }
            assembliesPath.Add(filePath);
         }
         ConsoleUtils.ShowNLinesOnConsole(Console.CursorTop - cursorTop +1, ConsoleColor.Black);
      }

   }
}