using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample27_CreateDerived : MonoBehaviour 
{
    // Usually in Unity, polymorphism is a rather annoying issue.
    // If you don't know what polymorphism is; http://en.wikipedia.org/wiki/Polymorphism_%28computer_science%29
    // Read up about it, it's VERY important!
    // To fix the issue Unity has with polymorphism, you can use the CreateDerived attribute with a base type that derive from ComponentMonoBehaviour;
    // This allows you to create instances directly from that field.
    [Inspect, CreateDerived]
    public AIExample_BaseClass[] myField;
}
