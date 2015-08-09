using UnityEngine;
using System.Collections;

[System.Flags]
public enum CollisionFlags
{
    None = 0x00000000,
    Wall = 0x00000001,
    Item = 0x00000002,
}

