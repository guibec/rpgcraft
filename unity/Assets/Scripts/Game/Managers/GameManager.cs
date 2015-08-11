using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;

using AdvancedInspector;

public class GameManager : MonoSingleton<GameManager>
{
    protected StateMachine m_fsm;

    public GameObject m_mainCharacter;
    public Camera m_mainCamera;
    
    public Texture m_tileTextureMap;

    /// <summary>
    /// This contains the list of all spawned chunks
    /// </summary>
    private readonly LinkedList<ChunkInfo> m_chunks = new LinkedList<ChunkInfo>();

    /// <summary>
    /// This is a map between active (x,y) pair and their respective chunks
    /// </summary>
    private readonly Dictionary<Vector2, ChunkInfo> m_posToChunks = new Dictionary<Vector2, ChunkInfo>();

    // the data component of the player

    public GameObject m_worldMapChunkPrefab;
    public Transform m_worldAnchorRoot;

    public Player MainPlayer { get; private set; }

    [Inspect, SerializeField]
    private float m_playerDeathTime = 5.0f;

    public float PlayerDeathTime
    {
        get { return m_playerDeathTime; }
        set { m_playerDeathTime = value; }
    }

    public GameManager()
    {
        m_fsm = new GameManagerState_Machine(this);
    }

    public void Start()
    {
        MainPlayer = m_mainCharacter.GetComponent<Player>();

        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                SpawnChunkAt(i, j);
            }
        }
    }

    protected override void OnInit()
    {
        base.OnInit();
        Application.LoadLevelAdditive("ui");
    }

    void SpawnChunkAt(int x, int y)
    {
        SpawnChunkAt(new Vector2(x, y));
    }

    void SpawnChunkAt(Vector2 chunkPos)
    {
        Vector3 spawnPos = m_worldAnchorRoot.transform.position;
        spawnPos.x += chunkPos.x*64;
        spawnPos.y += chunkPos.y*64;

        Quaternion spawnRot = m_worldAnchorRoot.transform.rotation;

        Stopwatch sw = Stopwatch.StartNew();
        GameObject chunkObj = (GameObject)Instantiate(m_worldMapChunkPrefab, spawnPos, spawnRot);

        //Material material = new Material(shader);
        //material.SetTexture("_MainTex", material.mainTexture);
        //material.SetTextureOffset("_MainTex", material.mainTextureOffset);
        //material.SetTextureScale("_MainTex", material.mainTextureScale);
        //mr.material = material;


        chunkObj.GetComponent<Renderer>().material.mainTexture = m_tileTextureMap;
        chunkObj.name = string.Format("WorldMapChunk({0},{1})", (int)chunkPos.x, (int)chunkPos.y);
        sw.Stop();
        UnityEngine.Debug.Log(string.Format("Mesh Instanciation tool {0}ms", sw.ElapsedMilliseconds));

        GenerationTemplate template = new GenerationTemplate();

        // Let's have mountains go from 0.1 to 1.0 depending on the distance from origin

        float distFromOrigin = (float)Math.Sqrt((double)chunkPos.SqrMagnitude());

        float percMountain = distFromOrigin/10f;
        if (percMountain > 1.0f)
            percMountain = 1.0f;

        template.patchTemplate.Add(new PatchTemplate(ETile.Forest, 0.30f, 0.2f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Desert, 0.20f, 1.0f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Tree, 0.20f, 0.1f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Mountain, percMountain, 0.3f));

        ChunkInfo chunkInfo = new ChunkInfo(chunkPos);
        chunkInfo.Generate(template);

        TileMap anotherTileMap = chunkObj.GetComponent<TileMap>();
        anotherTileMap.SourceChunk = chunkInfo;

        // update all mapping
        m_chunks.AddLast(chunkInfo);
        m_posToChunks[new Vector2(chunkPos.x, chunkPos.y)] = chunkInfo;
    }

    private void OnSideStep( Vector2 afterColPosition, Vector2 beforeInputPosition, Vector2 afterInputPosition)
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
                double newX;
                if (xDir < 0)
                {
                    newX = Math.Floor(beforeInputPosition.x + 0.5f) - 0.5f;
                }
                else
                {
                    newX = Math.Ceiling(beforeInputPosition.x - 0.5f) + 0.5f;
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
                    MainPlayer.SetPosition(afterSideStepCol);
                    return;
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
                double newY;
                if (yDir < 0)
                {
                    newY = Math.Floor(beforeInputPosition.y + 0.5f) - 0.5f;
                }
                else
                {
                    newY = Math.Ceiling(beforeInputPosition.y - 0.5f) + 0.5f;
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
                    MainPlayer.SetPosition(afterSideStepCol);
                    return;
                }
            }
            

            // Else, try closest
            // TODO - Also, in case both are possible, go toward the closest one first

        }
    }

	// Update is called once per frame
    protected override void OnUpdate()
	{
        base.OnUpdate();

        m_fsm.Update();

        UpdateAudio();

        // There is a dependency on the main player for input recal right now
        Vector3 oldPosition = MainPlayer.BeforeInputPos;
        Vector3 newPosition = MainPlayer.AfterInputPos;

        // take care of main player first
        Vector3 afterColPosition = UpdateCollision(MainPlayer, MainPlayer.LastPosition, newPosition);
        MainPlayer.SetPosition(afterColPosition);

        OnSideStep(afterColPosition, oldPosition, newPosition);

        // take care of all entities as well
        foreach (var entity in EntityManager.Instance.Entities)
        {
            if (entity is Player)
                continue;

            // Recalage on all Entity
            newPosition = entity.transform.position;
            afterColPosition = UpdateCollision(entity, entity.LastPosition, newPosition);
            entity.transform.position = afterColPosition;
        }

	    UpdateInput();
	    UpdateChunks();
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
            float tFirst, tLast;
            bool collide = CollisionCode.IntersectMovingBox2DBox2D(originalBox, ci.Box, displacement, new Vector2(0, 0), out tFirst, out tLast);

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
                    // touch ..
                    UnityEngine.Debug.Log("Touch " + ci.GameObject);

                    // send OnTouchEvent on Entity
                    // Note: This will probably send multiple Touch events to the same pair. We should accumulate these TouchEvent and then sweep them if we get the bug.
                    Entity entity2 = ci.GameObject.GetComponent<Entity>();

                    // TODO - Since UpdateCollisionInternal is called multiple times per frame, also when checking potential collision detection, we need a way to turn it on and off
                    // to avoid sending false positive, also to avoid sending multiple touches on the same frame
                    if (entity2 != null)
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

    private void UpdateChunks()
    {
        // figure out the position of the main character in (x, y) chunk,
        Vector2 playerPos = m_mainCharacter.transform.position;
        Vector2 chunkPos = World2Chunk(playerPos);

        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                Vector2 newChunkPos = chunkPos + new Vector2(i, j);
                if (!IsValidChunk(newChunkPos))
                {
                    SpawnChunkAt(newChunkPos);

                    // just do one chunk per frame
                    //return;
                }
            }
        }
    }

    public bool IsValidChunk(Vector2 chunkPos)
    {
        return m_posToChunks.ContainsKey(chunkPos);
    }

    /// <summary>
    /// Given a world position, return the corresponding mapped chunk
    /// </summary>
    /// <param name="worldPos"></param>
    /// <returns></returns>
    static public Vector2 World2Chunk(Vector2 worldPos)
    {
        double newX = (worldPos.x + 32f) / ChunkInfo.DefaultChunkWidth;
        newX = Math.Floor(newX);

        double newY = (worldPos.y + 32f) / ChunkInfo.DefaultChunkHeight;
        newY = Math.Floor(newY);

        return new Vector2((float)newX, (float)newY);
    }

    /// <summary>
    /// Given a chunk, return the corresponding worldposition
    /// </summary>
    /// <param name="chunk">The chunk to get the world from</param>
    /// <returns></returns>
    static public Vector2 Chunk2World(ChunkInfo chunk)
    {
        return new Vector2(chunk.ChunkPos.x * 64.0f, chunk.ChunkPos.y * 64.0f);
    }

    static public Vector2 Chunk2World(ChunkInfo chunk, int x, int y)
    {
        return Chunk2World(chunk) - new Vector2(ChunkInfo.DefaultChunkWidth / 2, ChunkInfo.DefaultChunkHeight / 2) + new Vector2(x+0.5f, y+0.5f);
    }

    public bool GetTileDataFromWorldPos(Vector2 worldPos, out ChunkInfo chunk, out int x, out int y)
    {
        Vector2 chunkIndices = World2Chunk(worldPos);
        Vector2 worldChunkPos = new Vector2(chunkIndices.x * ChunkInfo.DefaultChunkWidth, chunkIndices.y * ChunkInfo.DefaultChunkHeight);

        Vector2 relativePos = new Vector2(worldPos.x, worldPos.y) - worldChunkPos;
        relativePos.x += ChunkInfo.DefaultChunkWidth / 2;
        relativePos.y += ChunkInfo.DefaultChunkHeight / 2;

        ChunkInfo chunkInfo;
        if (m_posToChunks.TryGetValue(chunkIndices, out chunkInfo))
        {
            chunk = chunkInfo;
            x = (int)Math.Floor(relativePos.x);
            y = (int)Math.Floor(relativePos.y);
            return true;
        }

        chunk = null;
        x = 0;
        y = 0;
        return false;
    }

    public TileInfo GetTileFromWorldPos(Vector2 worldPos)
    {
        ChunkInfo info;
        int x, y;
        if (GetTileDataFromWorldPos(worldPos, out info, out x, out y))
        {
            return info.ReadSlotValue(x, y);
        }
        else
            return TileInfo.GetInvalid();
    }
}
