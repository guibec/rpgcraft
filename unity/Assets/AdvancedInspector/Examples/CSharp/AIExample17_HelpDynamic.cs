using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample17_HelpDynamic : MonoBehaviour
    {
        // Similar to Inspect or Descriptor, the Help attribute can also be dynamic;
        [Help("GetHelp")]
        public float myField;

        private HelpItem GetHelp()
        {
            if (myField > 0)
                return new HelpItem(HelpType.Info, "Change me to a negative value.");
            else if (myField < 0)
                return new HelpItem(HelpType.Warning, "Change me for positive!");
            else
                return new HelpItem(HelpType.Error, "Change me!!");
        }
    }
}