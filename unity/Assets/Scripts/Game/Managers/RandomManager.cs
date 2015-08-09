using UnityEngine;
using System.Collections;

public class RandomManager : MonoSingleton<RandomManager> 
{
    private RandomManager()
    {
        m_random = new System.Random(); // No deterministic seed for now
    }

    public int Next(int minValue, int maxValue)
    {
        return m_random.Next(minValue, maxValue);
    }

    public float Next(float minValue, float maxValue)
    {
        float range = maxValue - minValue;

        return (float)(m_random.NextDouble()) * range + minValue;
    }

    private System.Random m_random;
}
