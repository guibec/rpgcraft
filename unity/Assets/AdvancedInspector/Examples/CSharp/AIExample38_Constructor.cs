using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using AdvancedInspector;

public class AIExample38_Constructor : MonoBehaviour
{
    // In some case, you may want a class to have a custom constructor.
    // Usually, Unity is unable to invoke that constructor.
    [Serializable]
    public class CustomConstructor
    {
        [Inspect]
        public string value;

        public CustomConstructor(string text)
        {
            value = text;
        }
    }

    // Advanced Inspector allows you to create your own instance using the constructor attribute.
    // You may setup your object as you wish before returning it.
    [Inspect, Constructor("InvokeConstructor")]
    public CustomConstructor[] constructors;

    public CustomConstructor InvokeConstructor()
    {
        return new CustomConstructor("This was added in a constructor");
    }
}


