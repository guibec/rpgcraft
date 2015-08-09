using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using AdvancedInspector;

[AdvancedInspector]
public class AIExample34_TitleDynamic : MonoBehaviour
{
    // Similar to other IRuntimeAttribute, Title can be modified at runtime from a bound method.
    [Inspect, Title("GetTitle")]
    public FontStyle style = FontStyle.Bold;

    private TitleAttribute GetTitle()
    {
        return new TitleAttribute(style, "This Title Is Dynamic");
    }
}
