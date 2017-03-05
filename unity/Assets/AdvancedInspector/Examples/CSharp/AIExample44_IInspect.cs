using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample44_IInspect : MonoBehaviour, IInspect
    {
        public Material material;

        [Expandable(false)]
        public new Transform transform;

        public object[] Inspect
        {
            get { return new object[] { material, transform }; }
        }
    }
}
