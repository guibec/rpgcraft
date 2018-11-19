using UnityEngine;

[RequireComponent(typeof(Camera))]
public class FollowTarget : MonoBehaviour {

    // target to follow
    public GameObject m_target;
    public float m_tolerance = 0.7f; // how much we allow the target to go off-center

    private Camera m_camera;

    // Use this for initialization
    public void Start ()
    {
        m_camera = GetComponent<Camera>();
    }
    
    // Update is called once per frame
    public void Update () {

    }

    public void LateUpdate()
    {
        if (m_target == null)
            return;

        // prevent target from reaching a certain bound
        float verticalOrthoSize = m_camera.orthographicSize;
        float horizontalOrthoSize = verticalOrthoSize*(Screen.width/(float)Screen.height);

        Vector3 bottomLeft = m_target.transform.position - new Vector3(horizontalOrthoSize * m_tolerance, verticalOrthoSize * m_tolerance, 0f);
        Vector3 topRight = m_target.transform.position + new Vector3(horizontalOrthoSize * m_tolerance, verticalOrthoSize * m_tolerance, 0f);

        Vector3 newPos = transform.position;

        // check if target has gone too far, if so, clamp back position
        if (newPos.x > topRight.x)
            newPos.x = topRight.x;
        if (newPos.x < bottomLeft.x)
            newPos.x = bottomLeft.x;
        if (newPos.y > topRight.y)
            newPos.y = topRight.y;
        if (newPos.y < bottomLeft.y)
            newPos.y = bottomLeft.y;

        transform.position = newPos;

        //transform.position = new Vector3(m_target.transform.position.x, m_target.transform.position.y, transform.position.z);
    }
}
