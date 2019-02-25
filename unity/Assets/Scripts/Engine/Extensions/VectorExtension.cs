using UnityEngine;

public static class VectorExtension
{
    public static Vector2[] ToVector2Array(this Vector3[] v3)
    {
        return System.Array.ConvertAll(v3, GetV2fromV3);
    }

    public static Vector3[] ToVector3Array(this Vector2[] v2)
    {
        return System.Array.ConvertAll(v2, GetV3fromV2);
    }

    public static Vector2 GetV2fromV3(Vector3 v3)
    {
        return new Vector2(v3.x, v3.y);
    }

    public static Vector3 GetV3fromV2(Vector2 v2)
    {
        return new Vector3(v2.x, v2.y, 1.0f);
    }
}
