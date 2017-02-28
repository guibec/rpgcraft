using UnityEngine;
using System.Collections;

public class MonoSingleton<T> : MonoBehaviour where T : MonoBehaviour
{
    private static T m_instance;

    // Accessor used to create the Singleton in non-main scene
    private static T DebugCreate()
    {
        GameObject g = new GameObject("__" + typeof(T).ToString() + "__Dyn");
        g.AddComponent<T>();
        return g.GetComponent<T>();
    }

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
            if (m_instance == null) // create as needed
                m_instance = DebugCreate();

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
