using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample12_Group : MonoBehaviour
    {
        // Item can be grouped using the Group attribute;
        [Group("My First Group", 0, 0, 1)]
        public float myFirstField;

        [Group("My First Group")]
        public float MyFirstProperty
        {
            get { return myFirstField; }
            set { myFirstField = value; }
        }

        // Flagging it false stops the chaining.
        [Group("My First Group")]
        public void MyFirstMethod()
        {
            myFirstField++;
        }

        // Grouping is done with similar names.
        // The second parameter is the order in which the different groups are shown.
        [Group("My Second Group", 1, Description = "This is some extra text.")]
        public float mySecondField;

        [Group("My Second Group")]
        public float MySecondProperty
        {
            get { return mySecondField; }
            set { mySecondField = value; }
        }

        [Group("My Second Group")]
        public void MySecondMethod()
        {
            mySecondField++;
        }
    }
}