using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using AdvancedInspector;

using UnityEngine.Events;

[AdvancedInspector]
public class AIExample36_NoPicker : MonoBehaviour
{
    [Inspect, NoPicker]
    public Camera Camera
    {
        get { return Camera.main; }
    }
}
