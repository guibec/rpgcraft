using UnityEngine;
using System.Collections;

using AdvancedInspector;

// You don't need [AdvancedInspector] if the parent class has it.
public class AIExample_DerivedClassC : AIExample_BaseClass
{
    [Inspect]
    public string myDerivedStringC;
}
