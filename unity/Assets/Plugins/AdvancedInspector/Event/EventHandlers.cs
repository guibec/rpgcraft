using UnityEngine;

namespace AdvancedInspector
{
    public delegate void GenericEventHandler();

    public delegate void SenderEventHandler(object sender);

    public delegate void BoolEventHandler(object sender, bool value);
    public delegate void IntEventHandler(object sender, int value);
    public delegate void FloatEventHandler(object sender, float value);
    public delegate void StringEventHandler(object sender, string value);
    public delegate void ObjectEventHandler(object sender, object value);
    public delegate void Vector2EventHandler(object sender, Vector2 value);
    public delegate void Vector3EventHandler(object sender, Vector3 value);
    public delegate void Vector4EventHandler(object sender, Vector4 value);

    public delegate void CollisionEventHandler(object sender, Collision value);
    public delegate void ColliderEventHandler(object sender, Collider value);

    public delegate void ActionEventHandler(object sender, object[] args);
}