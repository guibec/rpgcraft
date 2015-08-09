using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using System.Collections;

using AdvancedInspector;

// Usually in Unity using the preview window at the bottom of the Inspector is a daunting task.
// With Advanced Inspector, you simply implement the IPrewiew interface... and that's it!
[AdvancedInspector]
public class AIExample28_Preview : MonoBehaviour, IPreview
{
    [Inspect]
    public UnityEngine.Object[] myPreviewList;

    // The IPreview interface only has one property getter;
    // If you pass null or an empty collection, the preview section is hidden.
    public UnityEngine.Object[] Preview
    {
        get { return myPreviewList; }
    }
}
