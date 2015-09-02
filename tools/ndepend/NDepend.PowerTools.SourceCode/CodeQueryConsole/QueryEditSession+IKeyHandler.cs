using System;


namespace NDepend.PowerTools.CodeQueryConsole {
   partial class QueryEditSession {
      interface IKeyHandler {
         bool IsHandlerFor(ConsoleKeyInfo consoleKeyInfo);
         State Handle(ConsoleKeyInfo consoleKeyInfo, State currentState);
      }

   }
}
