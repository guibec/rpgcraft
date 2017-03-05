using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample2_Inspect : MonoBehaviour
    {
        // The Inspector attribute is used to display something by the Advanced Inspector.
        // Even if it's not serialized
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

        // You can hide variable only for some specific mode.
        // You can right-click on a label and switch between mode.
        [Inspect(InspectorLevel.Debug)]
        public float showInDebug = 0;
    }
}