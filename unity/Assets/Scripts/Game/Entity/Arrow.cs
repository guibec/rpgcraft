using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Arrow : Entity
{
    public float m_throwSpeed = 25.0f;

    private readonly Hashtable m_hitEntities = new Hashtable();

    public float ThrowSpeed
    {
        get
        {
            return m_throwSpeed;
        }
    }
}
