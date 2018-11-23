using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AudioFadeInOut : MonoBehaviour {
    public void StartFadeInOut(AudioSource fadeOut, float timeFadeOut, AudioSource fadeIn, float timeFadeIn)
    {
        if (fadeOut != null)
        {
            StartCoroutine(FadeOut(fadeOut, timeFadeOut));
        }
    }

    private IEnumerator FadeOut(AudioSource audioSource, float fadeTime)
    {
        float startVolume = audioSource.volume;

        while (audioSource.volume > 0)
        {
            audioSource.volume -= TimeManager.Dt / fadeTime;
            audioSource.volume = Mathf.Max(audioSource.volume, 0);
            yield return null;
        }

        audioSource.Stop();
        audioSource.volume = startVolume;
    }
}
