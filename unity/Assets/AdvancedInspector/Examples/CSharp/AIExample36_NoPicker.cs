using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample36_NoPicker : MonoBehaviour
    {
        // No picker disable the picking tool on the left of an object field.
        [Inspect, NoPicker]
        public Camera Camera
        {
            get { return Camera.main; }
        }
    }
}