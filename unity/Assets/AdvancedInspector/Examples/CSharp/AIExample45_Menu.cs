using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample45_Menu : MonoBehaviour
    {
        [Menu("Reset To Zero", "ResetToZero")]
        public float value = 10;

        private void ResetToZero()
        {
            value = 0;
        }
    }
}
