using UnityEngine;
using System;

namespace AdvancedInspector
{
    public class AIExample14_Background : MonoBehaviour
    {
        // The background attribute is simply used to stored a color for the background of an expandable item.
        [Background(1, 0.5f, 0)]
        public ExpandableClass myObject;

        [Serializable]
        public class ExpandableClass
        {
            public float myField;
        }

        // It can also be dynamic.
        [Background("GetBackgroundColor")]
        public float[] myArray;

        public Color color;

        private Color GetBackgroundColor()
        {
            return color;
        }
    }
}