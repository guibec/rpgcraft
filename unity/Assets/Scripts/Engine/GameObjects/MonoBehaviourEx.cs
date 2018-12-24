using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MonoBehaviourEx : MonoBehaviour
{
    public GameObject Instantiate(GameObject prefab)
    {
        return MonoBehaviour.Instantiate(prefab);
    }

    public GameObject  Instantiate(GameObject parent, Vector3 position, Quaternion rotation)
    {
        return MonoBehaviour.Instantiate(parent, position, rotation);
    }
}
