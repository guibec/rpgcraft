using System;
using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class RandomManager : MonoSingleton<RandomManager> 
{
    private RandomManager()
    {
        m_random = new System.Random(); // No deterministic seed for now
    }

    public bool Boolean()
    {
        return Next(0, 2) == 0;
    }

    public int Next(int minValue, int maxValue)
    {
        return m_random.Next(minValue, maxValue);
    }

    public bool Probability(float perc)
    {
        if (perc >= 1.0f)
            return true;
        else if (perc <= 0.0f)
            return false;
        else
        {
            return Next(0, 1.0f) <= perc;
        }

    }

    public Vector2 Vector()
    {
        double angle = m_random.NextDouble()*Math.PI*2;
        return new Vector2((float)Math.Cos(angle), (float)Math.Sin(angle));
    }

    public float Next(float minValue, float maxValue)
    {
        float range = maxValue - minValue;

        return (float)(m_random.NextDouble()) * range + minValue;
    }

    private System.Random m_random;
}
