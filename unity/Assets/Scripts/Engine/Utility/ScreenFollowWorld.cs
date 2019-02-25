using UnityEngine;

/// <summary>
/// Used to make a UIObject follow a worldObject projected into UI space
/// The same distance is always kept relative to the parent
/// </summary>
public class ScreenFollowWorld : MonoBehaviourEx 
{
    private GameObject m_target;

    public GameObject Target 
    { 
        get
        {
            return m_target;
        }

        set
        {
            m_target = value;
            Offset = transform.position - Camera.main.WorldToScreenPoint(m_target.transform.position);
        }
    }

    public Vector2 Offset { get; set; }

    // Update is called once per frame
    void Update () 
    {
        if (Target != null)
        {
            Vector2 pos2d = Camera.main.WorldToScreenPoint(m_target.transform.position);
            pos2d += Offset;
            transform.position = pos2d;
        }
    }
}
