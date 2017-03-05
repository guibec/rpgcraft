using UnityEngine;

namespace AdvancedInspector
{
    // As of version 1.52, Unity attributes are also supported. 
    // So you will have to change your code as little as possible.
    public class AIExample0_UnityAttributes : MonoBehaviour
    {
        [Range(0, 10)]
        public int rangeField;

        [Header("This is a header")]
        public int headerField;

        [Tooltip("This is a tooltip")]
        public int tooltipField;

        [Space(10)]
        public int spaceField;

        [Multiline]
        public string multilineField;

        [Inspect, TextArea(3,7)]
        public string textAreaField;

        [Angle, RangeValue(0, 360)]
        public float wheel;
    }
}