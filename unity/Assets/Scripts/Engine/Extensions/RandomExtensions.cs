using UnityEngine;
using System.Collections;
using System.Runtime.CompilerServices;

namespace IronExtension
{
    public static class RandomExtension
    {
        public static void InPlaceShuffle(this int[] array)
        {
            int count = array.Length;
            for (int i = 0; i < count; ++i)
            {
                int source = i;
                int dest = Random.Range(i, count);
                int prev = array[source];
                array[source] = array[dest];
                array[dest] = prev;
            }
        }

        public static void InPlaceShuffle(this Vector2[] array)
        {
            InPlaceShuffle(array, array.Length);
        }

        public static void InPlaceShuffle(this Vector2[] array, int limit)
        {
            for (int i = 0; i < limit; ++i)
            {
                int source = i;
                int dest = Random.Range(i, limit);
                Vector2 prev = array[source];
                array[source] = array[dest];
                array[dest] = prev;
            }
        }
        public static int RandomValue(this Range<int> range)
        {
            return RandomManager.Next(range.Minimum, range.Maximum);
        }

        public static float RandomValue(this Range<float> range)
        {
            return RandomManager.Next(range.Minimum, range.Maximum);
        }
    }
}
