public class MonoSingleton<T> : MonoBehaviourEx where T : MonoBehaviourEx
{
    private static T m_instance;

    protected virtual void Awake()
    {
        if (!m_instance)
        {
            m_instance = (T) FindObjectOfType(typeof(T));
            DontDestroyOnLoad(gameObject);
            OnInit();
        }
        else
        {
            Destroy(gameObject);
        }
    }
    public static T Instance
    {
        get
        {
            return m_instance;
        }
    }

    protected virtual void OnInit()
    {

    }

    public void Update()
    {
        OnUpdate();
    }

    public void LateUpdate()
    {
        OnLateUpdate();
    }

    protected virtual void OnUpdate()
    {
        
    }

    protected virtual void OnLateUpdate()
    {
        
    }
}
