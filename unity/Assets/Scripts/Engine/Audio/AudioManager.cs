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

    private E_Music m_currentMusic = E_Music.None;

    public void PlayDig()
    {
        int indexToPlay=0;
        if (m_digAudio == null || m_digAudio.Count == 0)
            return;
        else if (m_digAudio.Count > 1)
            indexToPlay = Random.Range(0, m_digAudio.Count);

        AudioSource.PlayClipAtPoint(m_digAudio[indexToPlay], GameManager.Instance.m_mainCamera.transform.position);
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
