using UnityEngine;
using System.Collections;

/// <summary>
/// Represent an information about a collision
/// </summary>
public struct CollisionInfo
{
    public CollisionInfo(GameObject object_, Box2D box_, CollisionFlags flags_) : this()
    {
        GameObject = object_;
        Box = box_;
        Flags = flags_;
    }

    public GameObject GameObject { get; private set; }
    public Box2D Box { get; private set; }
    public CollisionFlags Flags { get; private set; }
}