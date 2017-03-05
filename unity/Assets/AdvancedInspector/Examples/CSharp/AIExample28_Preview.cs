using UnityEngine;

namespace AdvancedInspector
{
    // Usually in Unity using the preview window at the bottom of the Inspector is a daunting task.
    // With Advanced Inspector, you simply implement the IPrewiew interface... and that's it!
    public class AIExample28_Preview : MonoBehaviour, IPreview
    {
        public Object[] myPreviewList;

        // The IPreview interface only has one property getter;
        // If you pass null or an empty collection, the preview section is hidden.
        public Object[] Preview
        {
            get { return myPreviewList; }
        }
    }
}