using UnityEngine;
using System.Collections;

[System.Serializable]
public class Entity : MonoBehaviourEx 
{
    private bool m_destroying = false;
    private Vector3 m_lastPosition = Vector3.zero;
    protected StateMachine m_fsm;

    public int Id { private set; get; }

    public Entity()
    {
    }

    private void Start()
    {
        Id = EntityManager.Instance.Register(this);

        OnStart();
    }

    private void Awake()
    {
        OnAwake();
    }

    private void Update()
    {
        OnUpdate();
    }

    private void LateUpdate()
    {
        OnLateUpdate();
    }

    public virtual void OnTouch(Entity other)
    {
        // Also pass the information to the hurt touch component if there is one
        TouchComponent touchComponent = GetComponent<TouchComponent>();
        if (touchComponent)
        {
            touchComponent.OnTouch(other);
        }
    }

    public void RequestDestroy()
    {
        SpawnManager.Instance.OnKilled(this);
        m_destroying = true;
    }

    public bool IsDestroying()
    {
        return m_destroying;
    }

    protected virtual void OnStart()
    {
        
    }

    protected virtual void OnAwake()
    {
        LastPosition = transform.position;
    }

    protected virtual void OnUpdate()
    {
        if (m_fsm != null)
        {
            m_fsm.Update();    
        }
        

        if (m_destroying)
        {
            OnEntityDestroy();
        }
    }

    protected virtual void OnLateUpdate()
    {
        // We moved from LastPosition to transform.position
        Vector2 newPosition = transform.position;

        CollisionManager.Instance.OnLateUpdate(this, newPosition);

        LastPosition = newPosition;
    }

    // This is Unity destroy method.
    // I would much prefer we always control our life-span through our function that be "surprised" my object is now dead
    // should be called after everything have been taken care of
    private void OnDestroy()
    {
        //
    }

    protected virtual void OnEntityDestroy()
    {
        CollisionManager.Instance.OnDestroy(this);
        UnityEngine.Object.Destroy(gameObject);
        EntityManager.Instance.Unregister(this);
    }

    /// <summary>
    /// Set the position of the Entity. Entity will go through normal collision code and may refuse to be set to this position
    /// </summary>
    /// <param name="newPosition">New position of entity</param>
    public void SetPosition(Vector3 newPosition)
    {
        transform.position = newPosition;
    }

    /// <summary>
    /// Teleport Entity to position newPosition. No collision and no events are triggered by this move.
    /// </summary>
    /// <param name="newPosition">New position of entity</param>
    public void TeleportToPosition(Vector3 newPosition)
    {
        transform.position = newPosition;
        m_lastPosition = newPosition;
    }

    public Vector3 LastPosition
    {
        get => m_lastPosition;
        private set => m_lastPosition = value;
    }

    [SerializeField]
    private bool m_canBeKnockedBack = true;
    public bool CanBeKnockedBack
    {   get
        {
            return m_canBeKnockedBack;
        }

        set
        {
            m_canBeKnockedBack = value;
        }
    }

    public void KnockBack(Vector2 dir, float force, float time)
    {
        if (!CanBeKnockedBack)
        {
            return;
        }

        if (m_fsm.IsInState<EntityState_Live>())
        {
            EntityState_Live psl = m_fsm.FindStateByType<EntityState_Live>();
            if (psl != null)
            {
                psl.KnockBack(dir * force, time);
            }
        }
    }

    public virtual Box2D Box
    {
        get
        {
            return new Box2D(transform.position, 0.5f, 0.5f);    
        }
    }

    protected StateMachine FSM
    {
        get
        {
            return m_fsm;
        }
    }
}


