using UnityEngine;
using System.Collections;

public class Entity : MonoBehaviour 
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
        
    }

    public void RequestDestroy()
    {
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
        m_lastPosition = transform.position;
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
        LastPosition = transform.position;
    }

    // This is Unity destroy method.
    // I would much prefer we alway control our life-span through our function that be "surprised" my object is now dead
    // should be called after everything have been taken care of
    private void OnDestroy()
    {
        //
    }

    protected virtual void OnEntityDestroy()
    {
        DestroyObject(gameObject);        
        EntityManager.Instance.Unregister(this);
    }

    public void SetPosition(Vector3 newPosition)
    {
        transform.position = newPosition;
    }

    public Vector3 LastPosition
    {
        get
        {
            return m_lastPosition;
        }

        private set
        {
            m_lastPosition = value;
        }
    }

    public virtual Box2D Box2D
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
