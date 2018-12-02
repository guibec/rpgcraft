using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// Low-level class for Collision system
/// </summary>
public static class CollisionCode
{
    public static bool DrawCollision { get; set; }

    static CollisionCode()
    {
        DrawCollision = false;
    }

    private static List<CollisionInfo> m_cachedList = new List<CollisionInfo>();

    // return a list of all potential collision you may be going through
    public static CollisionInfo[] BroadPhase(Entity owner, Box2D sweep)
    {
        m_cachedList.Clear();

        // first, increase the sweep box so that it covers all the tile it is touching
        Box2D fullCover = sweep;
        fullCover.Left = Mathf.Floor(fullCover.Left);
        fullCover.Right = Mathf.Ceil(fullCover.Right);
        fullCover.Top = Mathf.Ceil(fullCover.Top);
        fullCover.Bottom = Mathf.Floor(fullCover.Bottom);

        // 0.9 is not a typo, it's just using a large epsilon which is currently fine because our sweep box is currently always at least one tile large
        DebugUtils.Assert(fullCover.Width >= 0.99);
        DebugUtils.Assert(fullCover.Height >= 0.99);

        if (Input.GetKeyDown(KeyCode.F10))
        {
            DrawCollision = !DrawCollision;
        }

        // check against world
        for (float i = fullCover.Left; i + 0.9f < fullCover.Right; i += 1.0f )
        {
            for (float j = fullCover.Bottom; j + 0.9f < fullCover.Top; j += 1.0f )
            {
                if (DrawCollision)
                {
                    DebugUtils.DrawRect(new Vector2(i, j), new Vector2(i + 1.0f, j + 1.0f), Color.blue);
                }

                Vector2 checkVector = new Vector2(i + 0.5f, j + 0.5f);

                TileInfo ti = GameManager.Instance.GetTileFromWorldPos(checkVector);
                ETile tile = ti.Tile;
                if (WorldTile.IsCollision(tile))
                {
                    Box2D box2d = new Box2D(i, j+1.0f, i + 1.0f, j);
                    m_cachedList.Add(new CollisionInfo(null, box2d, CollisionFlags.Wall));
                }

                // check entity as well
                // TODO: This does a double lookup from above
                List<Entity> entities = GameManager.Instance.GetEntitiesFromWorldPos(checkVector);
                if (entities == null)
                {
                    continue;
                }

                foreach (Entity entity in entities)
                {
                    // pair-wise check must be done once only to avoid obtaining multiple collisions
                    if (entity.Id <= owner.Id)
                    {
                        continue;
                    }

                    Box2D box2d = entity.Box;
                    if (entity is ItemInstance)
                    {
                        m_cachedList.Add(new CollisionInfo(entity, box2d, CollisionFlags.Item));
                    }
                    else
                    {
                        m_cachedList.Add(new CollisionInfo(entity, box2d, CollisionFlags.None));
                    }
                }
            }
        }

        // let's draw all collisions
        if (DrawCollision)
        {
            foreach (var ci in m_cachedList)
            {
                ci.Box.Draw(Color.white);
            }
        }

        return m_cachedList.ToArray();
    }

    public static bool TestPointBox2D(Vector2 p, Box2D b)
    {
        return (p.x >= b.Min(0) && p.x <= b.Max(0) && p.y >= b.Min(1) && p.y <= b.Max(1));
    }

    public static bool TestBox2DBox2D(Box2D a, Box2D b)
    {
        // Exit with no intersection if separated along an axis
        if (a.Max(0) < b.Min(0) || a.Min(0) > b.Max(0)) return false;
        if (a.Max(1) < b.Min(1) || a.Min(1) > b.Max(1)) return false;
        // Overlapping on all axes means AABBs are intersecting
        return true;
    }

    /// <summary>
    /// Check for collision between two dynamic moving boxes. Return collision information.
    /// </summary>
    /// <param name="a">The first box</param>
    /// <param name="b">The second box</param>
    /// <param name="va">The movement of the first box</param>
    /// <param name="vb">The movement of the second box</param>
    /// <param name="tIn">When collision in time occurs in term of ratio (0 to 1). Only used if function returns true.</param>
    /// <param name="tOut">When the collision does not occur anymore (0 to 1). Only used if function returns true. Will be set to 1 if still colliding at end of movement</param>
    /// <returns>If a collision occurred.</returns>
    public static bool IntersectMovingBox2DBox2D(Box2D a, Box2D b, Vector2 va, Vector2 vb, out float tIn, out float tOut)
    {
        // Initialize times of first and last contact
        tIn = 0f;
        tOut = 1f;

        float[] tFirst = { 0.0f, 0.0f };
        float[] tLast = { 1.0f, 1.0f };
        bool[] tContact = {false, false};

        if (TestBox2DBox2D(a, b))
        {
            tIn = tOut = 0.0f;
            return true;
        }
        else if (va.SqrMagnitude() < Vector2.kEpsilon && vb.SqrMagnitude() < Vector2.kEpsilon)
        {
            return false;
        }

        // Use relative velocity; effectively treating ’a’ as stationary
        Vector2 v = vb - va;

        // For each axis, determine times of first and last contact, if any
        for (int i = 0; i < 2; i++)
        {
            float velocity = i == 0 ? v.x : v.y;

            // check if they are already touching on that axis at the beginning
            if (a.Max(i) >= b.Min(i) && a.Min(i) <= b.Max(i))
            {
                tFirst[i] = 0f;
                tContact[i] = true;
            }

            if (velocity < 0.0f)
            {
                if (b.Max(i) < a.Min(i))
                {
                    return false; // Non intersecting and moving apart
                }

                if (a.Max(i) < b.Min(i))
                {
                    tFirst[i] = (a.Max(i) - b.Min(i)) / velocity;
                    tContact[i] = true;
                }

                if (b.Max(i) > a.Min(i))
                {
                    tLast[i] = (a.Min(i) - b.Max(i))/velocity;
                    tContact[i] = true;
                }
            }
            else if (velocity > 0.0f)
            {
                if (b.Min(i) > a.Max(i))
                {
                    return false; // Non intersecting and moving apart
                }

                if (b.Max(i) < a.Min(i))
                {
                    tFirst[i] = (a.Min(i) - b.Max(i))/velocity;
                    tContact[i] = true;
                }
                if (a.Max(i) > b.Min(i))
                {
                    tLast[i] = (a.Max(i) - b.Min(i))/velocity;
                    tContact[i] = true;
                }
            }


            if (!tContact[i])
            {
                return false;
            }
        }

        tIn = Mathf.Max(tFirst[0], tFirst[1]);
        tOut = Mathf.Min(tLast[0], tLast[1]);

        // No overlap possible if time of first contact occurs after time of last contact
        // also, we don't consider overlap if they occur in the future of this frame, or before this frame
        if (tIn > tOut || tIn > 1.0f || tIn < 0f)
        {
            return false;
        }

        //UnityEngine.Debug.Log(string.Format("Dynamic collision on tFirst={0} and tLast={0}", tIn, tOut));
        return true;

    }
}
