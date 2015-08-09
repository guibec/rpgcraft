using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using UnityEngine.Events;
using UnityEngine.UI;
using AdvancedInspector;

[AdvancedInspector]
public class AIExample37_UnityEvent : MonoBehaviour
{
    [Inspect]
    public UnityEvent testEvent;
}
