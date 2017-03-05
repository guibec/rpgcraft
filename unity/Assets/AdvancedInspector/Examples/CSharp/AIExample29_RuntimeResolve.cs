using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample29_RuntimeResolve : MonoBehaviour
    {
        public bool myBoolean;
        public float myFloat;
        public string myString;

        private int index = 0;

        // The RuntimeResolve attribute is an edge case tool where you want to display an object, but don't know it's proper type.
        // In this case, there is no editor for "object", but there's one for bool, float and string.
        // By default the Advanced Inspector uses the editor related to the declared type, not the current object type.
        // The RuntimeResolve forces it to seek the proper editor for the current type.
        // Resolving editors every refresh has an impact of performance. Use it with care.
        [Inspect, RuntimeResolve]
        public object MyProperty
        {
            get
            {
                if (index == 0)
                    return myBoolean;
                else if (index == 1)
                    return myFloat;
                else
                    return myString;
            }
            set
            {
                if (index == 0)
                    myBoolean = (bool)value;
                else if (index == 1)
                    myFloat = (float)value;
                else
                    myString = (string)value;
            }
        }

        [Inspect]
        public void PressMe()
        {
            index++;
            if (index > 2)
                index = 0;
        }
    }
}