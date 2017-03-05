using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample31_Range : MonoBehaviour
    {
        // RangeInt and RangeFloat are two struct that are used to offer a min-max range.
        // Those struct are used in conjunction with the RangeValue attribute.
        [RangeValue(0, 50)]
        public RangeInt myRangeInt = new RangeInt(10, 20);

        [RangeValue(-10f, 10f)]
        public RangeFloat myRangeFloat = new RangeFloat(-2.5f, 5.1f);
    }
}