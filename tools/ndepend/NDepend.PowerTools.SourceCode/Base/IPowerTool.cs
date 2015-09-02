

namespace NDepend.PowerTools.Base {
   interface IPowerTool {
      string Name { get; }
      string[] Description { get; }
      void Run();
   }
}
