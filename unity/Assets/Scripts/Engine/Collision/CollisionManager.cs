using UnityEngine;
using System.Collections;

/// <summary>
/// High-level class for collision system
/// </summary>
public class CollisionManager : MonoSingleton<CollisionManager> 
{
    public bool HasCollision(ChunkInfo ci, int x, int y)
    {
        Vector2 worldPos = GameManager.Chunk2World(ci, x, y);

        Box2D worldBox = new Box2D(worldPos, 0.5f, 0.5f);

        // Check against any player
        Box2D playerBox = GameManager.Instance.MainPlayer.Box;

        return CollisionCode.TestBox2DBox2D(worldBox, playerBox);
    }
}
