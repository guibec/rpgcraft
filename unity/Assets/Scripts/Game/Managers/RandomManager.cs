using System;
using UnityEngine;

public class RandomManager : MonoSingleton<RandomManager> 
{
    private static System.Random m_random = new System.Random();

    public static bool Boolean()
    {
        return Next(0, 2) == 0;
    }

    /// <summary>
    /// Return a random integer between minValue (included) and maxValue (not included)
    /// </summary>
    /// <param name="minValue"></param>
    /// <param name="maxValue"></param>
    /// <returns></returns>
    public static int Next(int minValue, int maxValue)
    {
        return m_random.Next(minValue, maxValue);
    }

    public static bool Probability(float perc)
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

    public static Vector2 Vector()
    {
        double angle = m_random.NextDouble()*Math.PI*2;
        return new Vector2((float)Math.Cos(angle), (float)Math.Sin(angle));
    }

    public static float Next(float minValue, float maxValue)
    {
        float range = maxValue - minValue;

        return (float)(m_random.NextDouble()) * range + minValue;
    }


}
