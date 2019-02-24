using UnityEngine;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// High-level class for collision system
/// </summary>
public class CollisionManager : MonoSingleton<CollisionManager> 
{
    public bool HasPlayerCollision(ChunkInfo ci, int x, int y)
    {
        Vector2 worldPos = WorldMap.Chunk2World(ci, x, y);

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

    static readonly Vector2[] neighbors = {
        new Vector2(0, 0),
        new Vector2(-1, 1),
        new Vector2(0, 1),
        new Vector2(1, 1),
        new Vector2(-1, 0),
        new Vector2(1, 0),
        new Vector2(-1, -1),
        new Vector2(0, -1),
        new Vector2(1, -1),
    };
    
    public void OnDestroy(Entity entity)
    {
        RemoveFromChunk(entity, entity.LastPosition);
    }

    public void OnLateUpdate(Entity entity, Vector2 newPosition)
    {
        Vector2 lastPosition = entity.LastPosition;

        // Need to check all 8 neighbors, including yourself.
        // TODO: This assumes the entity is not bigger than a single cell
        if (lastPosition == newPosition)
        {
            return;
        }

        RemoveFromChunk(entity, lastPosition);
        AddToChunk(entity, newPosition);
    }

    private void RemoveFromChunk(Entity entity, Vector2 lastPosition)
    {
        for (int i = 0; i < 9; i++)
        {
            GameManager.Instance.GetTileDataFromWorldPos(lastPosition + neighbors[i], out var chunkInfo, out var x, out var y);

            chunkInfo?.RemoveEntity(entity, x, y);
        }
    }

    private void AddToChunk(Entity entity, Vector2 newPosition)
    {
        for (int i = 0; i < 9; i++)
        {
            GameManager.Instance.GetTileDataFromWorldPos(newPosition + neighbors[i], out ChunkInfo chunkInfo, out int x, out int y);
            chunkInfo?.AddEntity(entity, x, y);
        }
    }

    /// <summary>
    /// Return iterator for all entities within a certain radius of another entity.
    /// Doesn't return itself
    /// </summary>
    /// <param name="source">Source entity</param>
    /// <param name="radius">Radius for checking</param>
    /// <returns></returns>
    public EntitiesWithinEntityRadiusEnumerator EntitiesWithinEntityRadius(Entity source, float radius)
    {
        return new EntitiesWithinEntityRadiusEnumerator(source, radius);
    }
}

public class EntitiesWithinEntityRadiusEnumerator : IEnumerable<Entity>
{
    public EntitiesWithinEntityRadiusEnumerator(Entity source, float radius)
    {
        m_source = source;
        m_radius = radius;
    }

    private readonly Entity m_source;
    private readonly float m_radius;

    public IEnumerator<Entity> GetEnumerator()
    {
        if (m_source == null || m_radius <= 0)
        {
            yield break;
        }

        foreach (var entity in EntityManager.Instance.Entities)
        {
            if (entity == m_source)
            {
                continue;
            }

            var entityPos = entity.transform.position;
            if ((entityPos - m_source.transform.position).sqrMagnitude <= m_radius * m_radius)
            {
                yield return entity;
            }
        }
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        return GetEnumerator();
    }
}

