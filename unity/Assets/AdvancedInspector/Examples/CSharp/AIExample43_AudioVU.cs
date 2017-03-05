using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// In recent version of Unity, a Audio VU bar is added at the bottom of the inspector to show how a filter is handling a sound stream.
    /// </summary>
    public class AIExample43_AudioVU : MonoBehaviour
    {
        [SerializeField]
        private float gain = 2;

        private void OnAudioFilterRead(float[] data, int channels)
        {
            for (int i = 0; i < data.Length; ++i)
                data[i] = data[i] * gain;
        }
    }
}
