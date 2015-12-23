using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public enum E_Music
{
    None,
    WorldMap,
}

public class AudioManager : MonoSingleton<AudioManager>
{
    public AudioSource m_worldMapMusic;

    // Like this for now, data driven later on
    public List<AudioClip> m_digAudio;
    public List<AudioClip> m_cutAudio;

    public List<AudioClip> m_hitAudio;

    private E_Music m_currentMusic = E_Music.None;

    public void PlayDig()
    {
        PlayAudioClip(m_digAudio);
    }

    public void PlayCut()
    {
        PlayAudioClip(m_cutAudio);
    }

    public void PlayHit()
    {
        PlayAudioClip(m_hitAudio);
    }

    static private void PlayAudioClip(List<AudioClip> possibilities)
    {
        int indexToPlay = 0;
        if (possibilities == null || possibilities.Count == 0)
            return;
        else if (possibilities.Count > 1)
            indexToPlay = Random.Range(0, possibilities.Count);

        AudioSource.PlayClipAtPoint(possibilities[indexToPlay], GameManager.Instance.m_mainCamera.transform.position);
    }

    public void PlayMusic(E_Music requestedMusic)
    {
        if (m_currentMusic == requestedMusic)
            return;

        m_currentMusic = requestedMusic;

        if (m_currentMusic == E_Music.None)
        {
            StopMusic();
        }
        else
        {
            switch (m_currentMusic)
            {
                case E_Music.WorldMap:
                    m_worldMapMusic.Play();
                    break;
                default:
                    Debug.Break();
                    break;
            }
        }
    }

    public void StopMusic() 
    {
        m_worldMapMusic.Stop();
    }

    public E_Music CurrentMusic
    {
        get
        {
            return m_currentMusic;
        }
    }
}
