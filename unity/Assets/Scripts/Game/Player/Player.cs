using System;
using UnityEngine;
using System.Collections;

public class Player : Entity 
{
    public Inventory Inventory { get; private set; }

    public Experience Experience { get; private set; }

    public Vector2 BeforeInputPos { get; set; }
    public Vector2 AfterInputPos { get; set; }

    public float m_playerSpeed = 8.0f;
    
    // Action information
    public enum EAction
    {
        None,
        Dig,
        Cut,
        Build,
    }

    public delegate void ActionCompletedDelegate(EntityAction completed_);

    public class EntityAction
    {
        // Action information
        public EAction m_action;    // What action are we performing ?
        public float m_startTime;   // When did we start this action ?
        public float m_duration;    // How long is this action supposed to last ?
        public Vector3 m_position;  // Where is the action being performed ?
        public ActionCompletedDelegate m_completedDelegate; 

        // internal house-keeping variables
        public float m_durationLeft;    // how much time left until the action is over ?
    }

    public EntityAction CurrentAction { get; private set; }
    public bool HasAction { get { return CurrentAction != null && CurrentAction.m_action != EAction.None; } }

    private EntityRender m_entityRender;

    public HealthComponent HealthComponent { get; private set; }

    public Player()
    {
        m_fsm = new PlayerStateMachine(this);
    }

    protected override void OnAwake()
    {
        base.OnAwake();
        Inventory = new Inventory(this);
        Experience = new Experience();

        m_entityRender = GetComponent<EntityRender>();
        HealthComponent = GetComponent<HealthComponent>();
    }

    private string[] frameGroups = 
    {
        "Up",
        "Down",
        "Left",
        "Right",
    };

    private const int Left_Cycle1 = 0;
    private const int Left_Cycle2 = 1;
    private const int Down_Cycle1 = 0;
    private const int Down_Cycle2 = 1;
    private const int Right_Cycle1 = 0;
    private const int Right_Cycle2 = 1;
    private const int Up_Cycle1 = 0;
    private const int Up_Cycle2 = 1;

    protected override void OnStart() 
    {
        base.OnStart();

        if (m_entityRender)
        {
            for (int i = 0; i < 8; ++i)
            {
                m_entityRender.SetFrameInfo(frameGroups[i / 2], i * 16, 48, 16, 16);
            }

            // now link frames together
            m_entityRender.LinkNextFrame(frameGroups[0], Up_Cycle1, Up_Cycle2);
            m_entityRender.LinkNextFrame(frameGroups[0], Up_Cycle2, Up_Cycle1);

            m_entityRender.LinkNextFrame(frameGroups[3], Right_Cycle1, Right_Cycle2);
            m_entityRender.LinkNextFrame(frameGroups[3], Right_Cycle2, Right_Cycle1);

            m_entityRender.LinkNextFrame(frameGroups[1], Down_Cycle1, Down_Cycle2);
            m_entityRender.LinkNextFrame(frameGroups[1], Down_Cycle2, Down_Cycle1);

            m_entityRender.LinkNextFrame(frameGroups[2], Left_Cycle1, Left_Cycle2);
            m_entityRender.LinkNextFrame(frameGroups[2], Left_Cycle2, Left_Cycle1);

            m_entityRender.SetGlobalFrameDelay(0.2f);

            m_entityRender.SetCurrentGroup(frameGroups[0]);

        }
    }

    public override void OnTouch(Entity other)
    {
        base.OnTouch(other);

        ItemInstance ii = other as ItemInstance;
        if (ii != null)
        {
            Inventory.Carry(ii.Item);
        }
    }

    // Update is called once per frame
    protected override void OnUpdate () 
    {
        base.OnUpdate();

	    if (HasAction)
        {
            EntityAction currentAction = CurrentAction;
            currentAction.m_durationLeft -= TimeManager.Dt;
            currentAction.m_durationLeft = Mathf.Max(currentAction.m_durationLeft, 0.0f);

            if (currentAction.m_durationLeft == 0.0f)
            {
                if (currentAction.m_completedDelegate != null)
                    currentAction.m_completedDelegate(currentAction);
                currentAction = null;
            }

            CurrentAction = currentAction;
        }

        // Enable auto-attack
        if (EntityManager.Instance.Count<SwordAttack>() == 0)
        {
            Enemy closest = null;
            float closestSqrDistance = float.MaxValue;
            foreach (Entity e in EntityManager.Instance.Entities)
            {
                Enemy candidate = e as Enemy;
                if (candidate == null)
                    continue;

                float sqrDistance = (candidate.transform.position - transform.position).sqrMagnitude;
                if (sqrDistance < closestSqrDistance)
                {
                    closest = candidate;
                    closestSqrDistance = sqrDistance;
                }
            }

            if (closest != null && closestSqrDistance <= 4.0f * 4.0f)
                SpawnAttackToward(gameObject.transform.position, closest.transform.position);
        }
    }

    protected override void OnLateUpdate()
    {
        Vector3 deltaPosition = transform.position - LastPosition;

        if (m_entityRender)
        {
            if (deltaPosition.x < 0)
            {
                m_entityRender.SetCurrentGroup("Left");
            }
            else if (deltaPosition.x > 0)
            {
                m_entityRender.SetCurrentGroup("Right");
            }
            else if (deltaPosition.y < 0)
            {
                m_entityRender.SetCurrentGroup("Down");
            }
            else if (deltaPosition.y > 0)
            {
                m_entityRender.SetCurrentGroup("Up");
            }
        }

        base.OnLateUpdate();
    }

    public bool CanReceiveDamage()
    {
        return HealthComponent.CanReceiveDamage();
    }

    public void ReceiveDamage(int damage)
    {
        HealthComponent.ReceiveDamage(damage);
    }

    private void StartAction(EAction action_, float duration_, Vector3 position_, ActionCompletedDelegate callback_)
    {
        if (HasAction)
            return;

        EntityAction newAction = new EntityAction {m_action = action_, m_startTime = Time.time};
        newAction.m_durationLeft = newAction.m_duration = duration_;
        newAction.m_position = position_;
        newAction.m_completedDelegate = callback_;

        CurrentAction = newAction;
    }

    // not sure if this should be using the action system
    // need to use object pooling here
    private void SpawnAttackToward(Vector2 origin, Vector2 target)
    {
        Vector2 dir = (target - origin).normalized;
        if (dir == Vector2.zero)
        {
            dir = Vector2.up; // Maybe use last movement direction...
        }

        GameObject prefab = EntityManager.Instance.m_attackPrefab;

        if (prefab == null)
        {
            UnityEngine.Debug.Log("Can't SpawnAttack since m_attackPrefab is null");
            return;
        }

        GameObject obj = Instantiate(prefab);

        if (obj == null)
            return;

        obj.transform.position = origin;

        EntityRender entityRender = obj.GetComponent<EntityRender>();
        entityRender.SetCurrentGroup("Main");

        Mover mover = obj.GetComponent<Mover>();
        Entity entity = obj.GetComponent<Entity>();
        if (mover && entity)
        {
            mover.StartInterpolation(origin + dir * 4.0f, 0.4f, () =>
            {
                entity.RequestDestroy();
            }
        );    
        }
        
    }

    // perform action at a given point, return true if done
    public bool ActionAt(Vector3 worldPos)
    {
        if (HasAction)
            return false;

        int selectedIndex = UIManager.Instance.SelectedInventorySlot;
        EItem selectedItem = Inventory.GetSlotInformation(selectedIndex).Item;

        if (selectedItem == EItem.Sword)
        {
            if (EntityManager.Instance.Count<SwordAttack>() > 0)
                return false;

            SpawnAttackToward(gameObject.transform.position, worldPos);
            return true;
        }

        // always compare against middle of tile
        worldPos.x = Mathf.Floor(worldPos.x) + 0.5f;
        worldPos.y = Mathf.Floor(worldPos.y) + 0.5f;
        worldPos.z = gameObject.transform.position.z;

        // minimum distance to allow digging
        float sqrDistance = (worldPos - gameObject.transform.position).sqrMagnitude;
        if (sqrDistance <= 6.0 * 6.0)
        {
            ChunkInfo chunkInfo;
            int x, y;
            bool success = GameManager.Instance.GetTileDataFromWorldPos(worldPos, out chunkInfo, out x, out y);

            TileInfo tileInfo = chunkInfo.ReadSlotValue(x, y);
            if (success)
            {
                if (selectedItem == EItem.PickAxe && tileInfo.Tile == ETile.Mountain)
                {
                    // Try to dig ! Start an action
                    StartAction(EAction.Dig, 0.05f, worldPos, actionCompleted_ =>
                    {
                        AudioManager.Instance.PlayDig();

                        // logic here for now, should be moved as we add more
                        TileInfo newTile = tileInfo.RemoveHP(10.0f);
                        if (newTile.HP == 0.0f)
                        {
                            newTile = newTile.TransformToTile(ETile.Grass);

                            SpawnManager.Instance.SpawnFromTile(tileInfo.Tile, chunkInfo, x, y);
                        }

                        chunkInfo.WriteSlotValue(x, y, newTile);
                    }
                    );

                    return true;
                }
                else if (selectedItem == EItem.Copper_Axe && tileInfo.Tile == ETile.Tree)
                {
                    // Try to dig ! Start an action
                    StartAction(EAction.Cut, 0.05f, worldPos, actionCompleted_ =>
                    {
                        AudioManager.Instance.PlayCut();

                        // logic here for now, should be moved as we add more
                        TileInfo newTile = tileInfo.RemoveHP(10.0f);
                        if (newTile.HP == 0.0f)
                        {
                            newTile = newTile.TransformToTile(ETile.Grass);

                            SpawnManager.Instance.SpawnFromTile(tileInfo.Tile, chunkInfo, x, y);
                        }

                        chunkInfo.WriteSlotValue(x, y, newTile);
                    }
                    );
                }
                else if (selectedItem == EItem.Stone && tileInfo.Tile != ETile.Mountain)
                {
                    if (CollisionManager.Instance.HasCollision(chunkInfo, x, y))
                        return false;

                    // Try to place a stone
                    StartAction(EAction.Build, 0.1f, worldPos, actionCompleted_ =>
                    {
                        // Make sure we can still perform action
                        if (CollisionManager.Instance.HasCollision(chunkInfo, x, y))
                            return;

                        AudioManager.Instance.PlayDig();

                        // logic here for now, should be moved as we add more
                        TileInfo newTile = tileInfo.TransformToTile(ETile.Mountain).MaxHP();
                        chunkInfo.WriteSlotValue(x, y, newTile);

                        Inventory.Use(selectedIndex);
                    }
                    );

                    return true;
                }

                return false;
            }
        }

        return false;
    }
}
