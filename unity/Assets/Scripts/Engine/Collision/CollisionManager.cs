using UnityEngine;
using System.Collections;
using System.Collections.Generic;

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

    public bool EnemyWithinPlayerRadius(float radius)
    {
        var playerPos = EntityManager.Instance.Player.transform.position;

        foreach (var entity in EntityManager.Instance.Entities)
        {
            if (!(entity is Enemy))
            {
                continue;
            }

            var entityPos = entity.transform.position;
            if ((entityPos - playerPos).sqrMagnitude <= radius * radius)
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Return iterator for all entities within a certain radius of another entity.
    /// Doesn't return itself
    /// </summary>
    /// <param name="source">Source entity</param>
    /// <param name="radius">Radius for checking</param>
    /// <returns></returns>
    public IEnumerator<Entity> EntitiesWithinEntityRadius(Entity source, float radius)
    {
        if (source == null || radius <= 0)
        {
            yield break;
        }
        
        foreach (var entity in EntityManager.Instance.Entities)
        {
            if (entity == source)
            {
                continue;
            }

            var entityPos = entity.transform.position;
            if ((entityPos - source.transform.position).sqrMagnitude <= radius * radius)
            {
                yield return entity;
            }
        }
    }
}
