using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample40_PropertyAttribute : MonoBehaviour
    {
        //PropertyAttribute drawer is a very limited system from Unity.
        //In most case, I would advice to write FieldEditor instead, as they are far more powerful.
        //Unlike FieldEditor, PropertyDrawers are only limited to fields. 
        //They cannot draw Property - even if their name would tell you otherwise - or Methods.
        //However, some other plugin rely on them, so support from AI is sometime needed.
        [PropertyAttributeExample]
        public float example;
    }
}