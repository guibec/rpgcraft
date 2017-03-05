using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample34_TitleDynamic : MonoBehaviour
    {
        // Similar to other IRuntimeAttribute, Title can be modified at runtime from a bound method.
        [Title("GetTitle")]
        public FontStyle style = FontStyle.Bold;

        private TitleAttribute GetTitle()
        {
            return new TitleAttribute(style, "This Title Is Dynamic");
        }
    }
}