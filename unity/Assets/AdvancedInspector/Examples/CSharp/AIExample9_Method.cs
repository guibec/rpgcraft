using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample9_Method : MonoBehaviour 
{
    // They show up as a button.
    [Inspect]
    public void MyMethod()
    {
        Debug.Log("You pressed my button");
    }

    // Using the Method attribute, with the Invoke variable, this function is called every time the inspector is refreshed.
    // This way, you can perform action or draw stuff on the inspector.
    [Inspect, Method(MethodDisplay.Invoke)]
    public void MyInvokedMethod()
    {
        GUILayout.Label("This was drawn from an invoked method.");
    }
}
