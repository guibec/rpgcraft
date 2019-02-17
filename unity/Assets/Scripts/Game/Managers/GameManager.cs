using System;
using UnityEngine;
using System.Collections.Generic;
using UnityEngine.Profiling;
using UnityEngine.SceneManagement;

public class GameManager : MonoSingleton<GameManager>
{
    protected StateMachine m_fsm;

    public GameObject m_mainCharacter;
    public Camera m_mainCamera;
    
    private WorldMap m_worldMap;

    // the data component of the player
    public Player MainPlayer { get; private set; }

    [/*Inspect,*/ SerializeField]
    private float m_playerDeathTime = 5.0f;

    public float PlayerDeathTime
    {
        get => m_playerDeathTime;
        set => m_playerDeathTime = value;
    }

    public WorldMap WorldMap => m_worldMap;

    // Map each Planet to a world.
    private string m_currentPlanet = "Earth";

    public string CurrentPlanet
    {
        get => m_currentPlanet;

        private set => m_currentPlanet = value;
    }

    public GameManager()
    {
        m_fsm = new GameManagerState_Machine(this);
    }

    protected override void Awake() 
    {
        base.Awake();

        m_worldMap = GetComponent<WorldMap>();
        TileMapping.BuildFromJSON("tilesInfo");
    }

    public void Start()
    {
        MainPlayer = m_mainCharacter.GetComponent<Player>();
        MainPlayer.TeleportToPosition(Vector3.zero);
        m_worldMap.Generate();
    }

    protected override void OnInit()
    {
        base.OnInit();
        SceneManager.LoadScene("ui", LoadSceneMode.Additive);
    }

    public void OnMainPlayerSpawn()
    {
        m_fsm.SwitchState<GameManagerState_Init>();
    }

    public void OnMainPlayerDead()
    {
        m_fsm.SwitchState<GameManagerState_Dead>();
    }

    public void RegenerateWorld()
    {
        m_worldMap.Generate();
        m_fsm.SwitchState<GameManagerState_Init>();
    }

    public void ChangePlanet(string planet)
    {
        if (planet == CurrentPlanet)
            return;

        m_worldMap.Load(planet);
        CurrentPlanet = planet;
    }

    public TileInfo GetTileFromWorldPos(Vector2 worldPos)
    {
        return m_worldMap.GetTileFromWorldPos(worldPos);
    }

    public bool GetTileDataFromWorldPos(Vector2 worldPos, out ChunkInfo chunk, out int x, out int y)
    {
        return m_worldMap.GetTileDataFromWorldPos(worldPos, out chunk, out x, out y);
    }

    public bool GetTileDataFromWorldPos(Vector2 worldPos, out ChunkInfo chunk, out Vector2 chunkPos)
    {
        bool result = GetTileDataFromWorldPos(worldPos, out chunk, out int x, out int y);
        chunkPos.x = x;
        chunkPos.y = y;
        return result;
    }

    private Vector2 OnSideStep( Vector2 afterColPosition, Vector2 beforeInputPosition, Vector2 afterInputPosition)
    {
        // not sure if this code should go here or within UpdateCollision
        // placing code here for now, we will see how it generalizes
        // make it easier to turn around edge/corner/enter small place

        // the total distance to consume
        double distance = (afterInputPosition - beforeInputPosition).magnitude;

        // we didn't go up/down, but we tried to
        if (afterColPosition.y == beforeInputPosition.y && (afterInputPosition.y > beforeInputPosition.y || afterInputPosition.y < beforeInputPosition.y))
        {
            for (int xDir = -1; xDir <= 1; xDir += 2)
            {
                // don't try to side-step in a direction the player specifically asked not go to
                if (afterInputPosition.x > beforeInputPosition.x) // we tried to go right
                {
                    if (xDir < 0)
                        continue;
                }
                else if (afterInputPosition.x < beforeInputPosition.x) // we tried to go left
                {
                    if (xDir > 0)
                        continue;
                }

                // 23.7 -> 23.5
                // 23.7 -> 24.5
                // which is good
                // But when we get an exact number
                // 23.5 -> 23.5
                // 23.5 -> 24.5
                // We are biased toward one direction
                // Right now we are solving this by fixing the value with newX -= 1 / newX += 1
                // This prevent the character from getting stuck not moving when near a hole
                // however, it also prevents the character from going inside the hole, which someone might expect

                double newX;
                if (xDir < 0)
                {
                    newX = Math.Floor(beforeInputPosition.x + 0.5f) - 0.5f;
                    if (newX == beforeInputPosition.x)
                        newX -= 1;
                }
                else
                {
                    newX = Math.Ceiling(beforeInputPosition.x - 0.5f) + 0.5f;
                    if (newX == beforeInputPosition.x)
                        newX += 1;
                }

                Vector2 sidePosition = new Vector2((float) newX, beforeInputPosition.y);
                Vector2 afterSideStepCol = UpdateCollision(MainPlayer, afterColPosition, sidePosition);

                // given that new position, could we actually continue where we wanted ?
                float yDelta = (afterInputPosition.y > beforeInputPosition.y) ? 0.2f : -0.2f;
                Vector2 goingDeeper = UpdateCollision(MainPlayer, afterSideStepCol,
                    new Vector2(afterSideStepCol.x, afterSideStepCol.y + yDelta));

                // if we were to go toward that direction, we would make it
                // now, calculate how much we can actually do in a frame
                if (yDelta > 0 && goingDeeper.y > afterSideStepCol.y || yDelta < 0 && goingDeeper.y < afterSideStepCol.y)
                {
                    // so we can do it, figure out really how much to go forward then
                    double sideStepDistance = Math.Abs(newX - beforeInputPosition.x);

                    if (sideStepDistance > distance)
                    {
                        Vector2 sideDir = (sidePosition - beforeInputPosition);
                        sideDir.Normalize();

                        sidePosition = beforeInputPosition + (sideDir * (float)distance);
                    }

                    afterSideStepCol = UpdateCollision(MainPlayer, afterColPosition, sidePosition);

                    // TODO - Use remaining momentum to move
                    return afterSideStepCol;
                }
            }
        }

        // left/right
        if (afterColPosition.x == beforeInputPosition.x && (afterInputPosition.x > beforeInputPosition.x || afterInputPosition.x < beforeInputPosition.x))
        {
            // Should we go up or down ?
            // Naive algorithm: Use shortest distance
            // Better algorithm, go up/down if you can go to the wanted direction afterward, otherwise go to the shortest one

            //
            // 13.5001 up to 14.4999 -> 14.5
            // ceil(13.5001 - 0.5) + 0.5 = ceil(13.001) + 0.5 -> 14 + 0.5 = 14.5
            // ceil(14.4999 - 0.5) + 0.5 = ceil(13.999) + 0.5 -> 14 + 0.5 = 14.5

            // yDir will be -1, then +1
            for (int yDir = -1; yDir <= 1; yDir += 2)
            {
                // don't try to side-step in a direction the player specifically asked not go to
                if (afterInputPosition.y > beforeInputPosition.y) // we tried to go up
                {
                    if (yDir < 0)
                        continue;
                }
                else if (afterInputPosition.y < beforeInputPosition.y) // we tried to go down
                {
                    if (yDir > 0)
                        continue;
                }

                double newY;
                if (yDir < 0)
                {
                    newY = Math.Floor(beforeInputPosition.y + 0.5f) - 0.5f;
                    if (newY == beforeInputPosition.y)
                        newY -= 1;
                }
                else
                {
                    newY = Math.Ceiling(beforeInputPosition.y - 0.5f) + 0.5f;
                    if (newY == beforeInputPosition.y)
                        newY += 1;
                }

                Vector2 sidePosition = new Vector2(beforeInputPosition.x, (float)newY);
                Vector2 afterSideStepCol = UpdateCollision(MainPlayer, afterColPosition, sidePosition);

                // given that new position, could we actually continue where we wanted ?
                float xDelta = (afterInputPosition.x > beforeInputPosition.x) ? 0.2f : -0.2f;
                Vector2 goingDeeper = UpdateCollision(MainPlayer, afterSideStepCol,
                    new Vector2(afterSideStepCol.x + xDelta, afterSideStepCol.y));

                // if we were to go toward that direction, we would make it
                // now, calculate how much we can actually do in a frame
                if (xDelta > 0 && goingDeeper.x > afterSideStepCol.x || xDelta < 0 && goingDeeper.x < afterSideStepCol.x)
                {
                    // so we can do it, figure out really how much to go forward then
                    double sideStepDistance = Math.Abs(newY - beforeInputPosition.y);

                    if (sideStepDistance > distance)
                    {
                        Vector2 sideDir = (sidePosition - beforeInputPosition);
                        sideDir.Normalize();

                        sidePosition = beforeInputPosition + (sideDir*(float) distance);
                    }

                    afterSideStepCol = UpdateCollision(MainPlayer, afterColPosition, sidePosition);

                    // TODO - Use remaining momentum to move
                    return afterSideStepCol;
                }
            }
            
            // Else, try closest
            // TODO - Also, in case both are possible, go toward the closest one first
        }

        return afterColPosition;
    }

    // Update is called once per frame
    protected override void OnUpdate()
    {
        base.OnUpdate();

        m_fsm.Update();

        UpdateAudio();

        // There is a dependency on the main player for input recal right now
        Profiler.BeginSample("Collision");
        Vector3 oldPosition = MainPlayer.BeforeInputPos;
        Vector3 newPosition = MainPlayer.AfterInputPos;

        // take care of main player first
        Vector3 afterColPosition = UpdateCollision(MainPlayer, MainPlayer.LastPosition, newPosition);
        MainPlayer.SetPosition(afterColPosition);

        Vector3 afterSideStepPosition = OnSideStep(afterColPosition, oldPosition, newPosition);
        MainPlayer.SetPosition(afterSideStepPosition);

        // take care of all entities as well
        foreach (var entity in EntityManager.Instance.Entities)
        {
            if (entity is Player)
            {
                continue;
            }

            // Recalage on all Entity
            newPosition = entity.transform.position;

            // Early optimization, take care of moving objects only
            // if you haven't moved, and you're still touching someone, you probably touched them last frame
            //if (entity.LastPosition == newPosition)
            //{
            //    continue;
            //}

            Profiler.BeginSample("UpdateCollison " + entity.name);
            afterColPosition = UpdateCollision(entity, entity.LastPosition, newPosition);
            Profiler.EndSample();
            entity.transform.position = afterColPosition;
        }
        Profiler.EndSample();

        UpdateInput();
        m_worldMap.OnUpdate(m_mainCharacter.transform.position);
    }

    protected override void OnLateUpdate()
    {
        base.OnLateUpdate();
    }

    private void UpdateAudio()
    {
        E_Music currentMusic = AudioManager.Instance.CurrentMusic;
        if (currentMusic == E_Music.None)
            AudioManager.Instance.PlayMusic(E_Music.WorldMap);
    }

    private void UpdateInput()
    {
        if (Input.GetKey("escape"))
        {
            Application.Quit();
        }

        // Specific action before
        if (Input.GetMouseButtonDown(0))
        {
            Vector3 worldPos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            if (MainPlayer.SkillActionAt(worldPos))
                return;
        }

        // Digging
        if (Input.GetMouseButton(0))
        {
            // check if player can consume it
            Vector3 worldPos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            if (MainPlayer.ActionAt(worldPos))
                return;
        }

        KeyCode[] keycodes =
        {
            KeyCode.Alpha1,
            KeyCode.Alpha2,
            KeyCode.Alpha3,
            KeyCode.Alpha4,
            KeyCode.Alpha5,
            KeyCode.Alpha6,
            KeyCode.Alpha7,
            KeyCode.Alpha8,
            KeyCode.Alpha9,
            KeyCode.Alpha0,
        };

        int index = 0;
        foreach (var keycode in keycodes)
        {
            if (Input.GetKeyDown(keycode))
            {
                UIManager.Instance.SelectedInventorySlot = index;
                break;
            }
            index++;
        }
    }

    private void UpdatePositionInternal(Entity entity, Vector3 oldPosition, ref Vector3 newPosition, int step)
    {
        // Figure out sweep area
        Vector2 oldPosition2D = new Vector2(oldPosition.x, oldPosition.y);
        Vector2 newPosition2D = new Vector2(newPosition.x, newPosition.y);

        float mostLeft = Math.Min(oldPosition2D.x, newPosition2D.x) - 0.5f;
        float mostRight = Math.Max(oldPosition2D.x, newPosition2D.x) + 0.5f;
        float mostUp = Math.Max(oldPosition2D.y, newPosition2D.y) + 0.5f;
        float mostDown = Math.Min(oldPosition2D.y, newPosition2D.y) - 0.5f;

        Vector2 topLeft = new Vector2(mostLeft, mostUp);
        Vector2 bottomRight = new Vector2(mostRight, mostDown);

        Box2D sweep = new Box2D(topLeft, bottomRight);

        Box2D originalBox = new Box2D(oldPosition2D, 0.5f, 0.5f);
        Box2D endBox = new Box2D(newPosition2D, 0.5f, 0.5f);

        Vector2 displacement = endBox.Center - originalBox.Center;

        // now get the list of tiles that we may have encountered during this sweep and return their boxes
        CollisionInfo[] broadPhase = CollisionCode.BroadPhase(entity, sweep);

        float bestT = 1.0f; // no collision
        foreach (CollisionInfo ci in broadPhase)
        {
            // now for each collision potential collision, figure out at which 't' we hit the box, return the shortest t in case of multiple collision
            bool collide = CollisionCode.IntersectMovingBox2DBox2D(originalBox, ci.Box, displacement, new Vector2(0, 0), out var tFirst, out var tLast);

            if (collide)
            {
                // Are we allowed to go through ?
                if ((ci.Flags & CollisionFlags.Wall) == CollisionFlags.Wall)
                {
                    // hard collision, prevent movement
                    bestT = Mathf.Min(tFirst, bestT);    
                }
                else
                {
                    // send OnTouchEvent on Entity
                    // Note: This will probably send multiple Touch events to the same pair. We should accumulate these TouchEvent and then sweep them if we get the bug.
                    Entity entity2 = ci.Entity;

                    // TODO - Since UpdateCollisionInternal is called multiple times per frame, also when checking potential collision detection, we need a way to turn it on and off
                    // to avoid sending false positive, also to avoid sending multiple touches on the same frame
                    if (entity2 != null && !entity2.IsDestroying())
                    {
                        entity.OnTouch(entity2);
                        entity2.OnTouch(entity);
                    }
                }
            }
        }

        if (bestT < 1.0f) // got at least one collision, reduce wanted velocity
        {
            displacement *= bestT;
            newPosition2D = oldPosition2D + displacement;
        }

        if (bestT >= 0.999)
        {
            newPosition.x = newPosition2D.x;
            newPosition.y = newPosition2D.y;
        }
        else if (step < 5)
        {
            // we were at oldPosition, we wanted to go at newPosition, we are currently stuck at newPosition2D

            // if displacement was on a single-axis, forget it, there's nothing we do can
            Vector3 wantedDisplacement = newPosition - oldPosition;
            if (Mathf.Abs(wantedDisplacement.x) <= 0.001 || Mathf.Abs(wantedDisplacement.y) <= 0.001)
            //if (true)
            {
                newPosition.x = newPosition2D.x;
                newPosition.y = newPosition2D.y;
            }
            else
            {
                Vector3 subOldPosition = new Vector3(newPosition2D.x, newPosition2D.y, 0.0f);

                Vector3 subNewPositionInX = newPosition;
                subNewPositionInX.y = subOldPosition.y;

                Vector3 subNewPositionInY = newPosition;
                subNewPositionInY.x = subOldPosition.x;

                // Try X-Axis only
                UpdatePositionInternal(entity, subOldPosition, ref subNewPositionInX, step + 1);

                // Try Y-Axis only
                UpdatePositionInternal(entity, subOldPosition, ref subNewPositionInY, step + 1);

                // Which one got us closer to our goal ?
                if ((subNewPositionInX - newPosition).sqrMagnitude < (subNewPositionInY - newPosition).sqrMagnitude)
                {
                    // TODO - Recurse again ?

                    // it was better to go in X only
                    newPosition.x = subNewPositionInX.x;
                    newPosition.y = subNewPositionInX.y;
                }
                else
                {
                    newPosition.x = subNewPositionInY.x;
                    newPosition.y = subNewPositionInY.y;
                }
            }
        }
    }

    /// <summary>
    /// Perform recalage on position
    /// </summary>
    /// <param name="entity">The entity to move</param>
    /// <param name="oldPosition">The previous position</param>
    /// <param name="newPosition">The new wanted position</param>
    /// <returns>The actual position after recalage</returns>
    private Vector3 UpdateCollision(Entity entity, Vector3 oldPosition, Vector3 newPosition)
    {
        UpdatePositionInternal(entity, oldPosition, ref newPosition, 1);
        return newPosition;
    }


    public List<Entity> GetEntitiesFromWorldPos(Vector2 worldPos)
    {
        if (GetTileDataFromWorldPos(worldPos, out var info, out var x, out var y))
        {
            return info.GetEntities(x, y);
        }
        else
        {
            return null;
        }
    }

    public ItemInstance GetItemInstanceFromWorldPos(Vector2 worldPos)
    {
        // (O(n)) - No spatial partitioning, will probably be slow when many items are present.
        foreach (ItemInstance item in ItemManager.Instance)
        {
            if (CollisionCode.TestPointBox2D(worldPos, item.Box))
            {
                return item;
            }
        }
        return null;
    }

    public Enemy GetEnemyFromWorldPos(Vector2 worldPos)
    {
        foreach (Entity entity in EntityManager.Instance.Entities)
        {
            Enemy enemy = entity as Enemy;
            if (enemy && CollisionCode.TestPointBox2D(worldPos, enemy.Box))
            {
                return enemy;
            }
        }
        return null;
    }
}
