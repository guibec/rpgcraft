using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample33_Title : MonoBehaviour
    {
        [SerializeField, HideInInspector]
        private int value = 10;

        // Title is similar to Unity header, but it can be place on method and properties.
        [Inspect, Title(FontStyle.Bold, "This is a line.\nThis is another line.")]
        public int Value
        {
            get { return value; }
        }
    }
}