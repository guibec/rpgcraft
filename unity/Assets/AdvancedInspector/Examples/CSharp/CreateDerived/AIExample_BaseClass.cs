namespace AdvancedInspector
{
    // It's very important for object that are created from the inspector to derive from ComponentMonoBehaviour instead of MonoBehaviour.
    [AdvancedInspector(true, true)]
    public abstract class AIExample_BaseClass : ComponentMonoBehaviour
    {
        [Inspect]
        public float myFloat;
    }
}