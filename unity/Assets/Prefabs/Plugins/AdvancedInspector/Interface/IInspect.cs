namespace AdvancedInspector
{
    /// <summary>
    /// Implements a property that give the Inspect object to create editor for.
    /// Allows to inspect other object, like the MeshRenderer showing materials at the bottom.
    /// </summary>
    public interface IInspect
    {
        /// <summary>
        /// In this case, it's not limited to UnityEngine.Object.
        /// You can pass any object.
        /// Pass null to remove the currently inspected object.
        /// </summary>
        object[] Inspect { get; }
    }
}