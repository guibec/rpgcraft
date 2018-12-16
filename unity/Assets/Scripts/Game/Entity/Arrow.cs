using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Arrow : Entity
{
    public float m_throwSpeed = 25.0f;

    public float ThrowSpeed
    {
        get
        {
            return m_throwSpeed;
        }
    }
}
