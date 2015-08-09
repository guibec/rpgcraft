using UnityEngine;
using System.Collections;

using AdvancedInspector;

// It's very important for object that are created from the inspector to derive from ComponentMonoBehaviour instead of MonoBehaviour.
[AdvancedInspector] 
public abstract class AIExample_BaseClass : ComponentMonoBehaviour
{
    [Inspect]
    public float myFloat;
}
