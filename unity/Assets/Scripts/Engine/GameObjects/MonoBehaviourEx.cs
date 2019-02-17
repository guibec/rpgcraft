using UnityEngine;

public class MonoBehaviourEx : MonoBehaviour
{
    static GameObject m_dynHolder;

    public GameObject Instantiate(GameObject prefab)
    {
        return Instantiate(prefab, Vector3.zero, Quaternion.identity);
    }

    public GameObject  Instantiate(GameObject prefab, Vector3 position, Quaternion rotation)
    {
        if (m_dynHolder == null)
        {
            m_dynHolder = new GameObject("_Dyn");
            m_dynHolder.transform.SetPositionAndRotation(Vector3.zero, Quaternion.identity);
        }

        GameObject gameObject = MonoBehaviour.Instantiate(prefab, position, rotation);
        if (gameObject != null)
        {
            gameObject.transform.SetParent(m_dynHolder.transform);
        }

        return gameObject;
    }
}
