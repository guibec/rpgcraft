using UnityEngine;

public class DebugUtils 
{ 
    //[Conditional("DEBUG")] 
    public static void Assert(bool condition) 
    { 
        if (!condition)
        {
            UnityEngine.Debug.LogError("Assert!");
        }
    }

    //[Conditional("DEBUG")]
    public static void DrawRect(Vector2 topLeft, Vector2 bottomRight, Color c)
    {
        Vector3 topLeft3D = new Vector3(topLeft.x, topLeft.y, 0.0f);
        Vector3 topRight3D = new Vector3(bottomRight.x, topLeft.y, 0.0f);
        Vector3 bottomLeft3D = new Vector3(topLeft.x, bottomRight.y, 0.0f);
        Vector3 bottomRight3D = new Vector3(bottomRight.x, bottomRight.y, 0.0f);

        UnityEngine.Debug.DrawLine(topLeft3D, topRight3D, c, 0.0f, false);
        UnityEngine.Debug.DrawLine(topRight3D, bottomRight3D, c, 0.0f, false);
        UnityEngine.Debug.DrawLine(bottomRight3D, bottomLeft3D, c, 0.0f, false);
        UnityEngine.Debug.DrawLine(bottomLeft3D, topRight3D, c, 0.0f, false);
    } 
}