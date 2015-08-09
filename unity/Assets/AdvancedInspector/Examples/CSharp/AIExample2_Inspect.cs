using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample2_Inspect : MonoBehaviour 
{
    // The Inspector attribute is used to display something by the Advanced Inspector.
    // By default, everything is hidden.
    [Inspect]
    public float myField;

    // Properties can also be inspected.
    [Inspect]
    public float MyProperty
    {
        get { return myField; }
        set { myField = value; }
    }

    // Method/Functions without input parameters can also be inspected.
    // They show up as a button.
    [Inspect]
    public void MyMethod()
    {
        myField++;
    }
}
