/// <summary>
/// Represent an information about a collision
/// </summary>
public struct CollisionInfo
{
    public CollisionInfo(Entity entity_, Box2D box_, CollisionFlags flags_) : this()
    {
        Entity = entity_;
        Box = box_;
        Flags = flags_;
    }

    public Entity Entity { get; }
    public Box2D Box { get; }
    public CollisionFlags Flags { get; }
}