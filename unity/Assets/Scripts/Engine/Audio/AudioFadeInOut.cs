using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AudioFadeInOut : MonoBehaviour {
    public void StartFadeOutIn(AudioSource fadeOut, float timeFadeOut, AudioSource fadeIn, float timeFadeIn)
    {
        StartCoroutine(FadeOutIn(fadeOut, timeFadeOut, fadeIn, timeFadeIn));
    }

    private IEnumerator FadeOutIn(AudioSource fadeOut, float timeFadeOut, AudioSource fadeIn, float timeFadeIn)
    {
        if (fadeOut != null)
        {
            yield return FadeOut(fadeOut, timeFadeOut);
        }

        if (fadeIn != null)
        {
            yield return FadeIn(fadeIn, timeFadeIn);
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

    private IEnumerator FadeIn(AudioSource audioSource, float fadeTime)
    {
        float originalVolume = audioSource.volume;
        audioSource.volume = 0;
        audioSource.Play();

        while (audioSource.volume < originalVolume)
        {
            audioSource.volume += TimeManager.Dt / fadeTime;
            audioSource.volume = Mathf.Min(audioSource.volume, originalVolume);
            yield return null;
        }
    }
}
