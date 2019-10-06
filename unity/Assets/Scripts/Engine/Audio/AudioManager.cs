using UnityEngine;
using System.Collections.Generic;

public enum E_Music
{
    None,
    WorldMap,
    Battle,
    BossBattle
}

[System.Serializable]
public struct MusicSlot
{
    public E_Music tag;
    public AudioSource source;
    public bool restorePosition;
    [HideInInspector] public float lastPosition;
}

[System.Serializable]
public enum E_Sound
{
    Dig,
    Cut,
    Hit,
}

[System.Serializable]
public struct SoundSlot
{
    public E_Sound tag;
    public List<AudioClip> clips;
}

[RequireComponent(typeof(AudioFadeInOut))]
public class AudioManager : MonoSingleton<AudioManager>
{
    public float m_musicVolume = 0.5f;
    public float m_sfxVolume = 1.0f;

    public float m_timeFadeInOut = 0.5f;

    public MusicSlot[] m_musics;
    private E_Music m_currentMusic = E_Music.None;
        
    public SoundSlot[] m_sounds = new SoundSlot[System.Enum.GetNames(typeof(E_Sound)).Length];
   
    private AudioFadeInOut m_audioFadeInOut;
    protected override void Awake()
    {
        m_audioFadeInOut = gameObject.GetComponent<AudioFadeInOut>();
        DebugUtils.Assert(m_audioFadeInOut != null);

        base.Awake();
    }

    void OnValidate()
    {
        if (m_sounds.Length != System.Enum.GetNames(typeof(E_Sound)).Length)
        {
            Debug.LogError("Sound entries differ [" + m_sounds.Length + "] It should be [" + System.Enum.GetNames(typeof(E_Sound)).Length + "]");
        }
        
        for (int index = 0; index < m_sounds.Length; ++index)
        {            
            if (m_sounds[index].tag != (E_Sound)index)
            {
                Debug.LogError("Invalid sound element " + index + ". It should be: [" + (E_Sound)index + "] not [" + m_sounds[index].tag + "]");
            }
        }
    }

    void OnEnable()
    {
        MusicVolume = m_musicVolume;
    }

    public float MusicVolume
    {
        get
        {
            return m_musicVolume;
        }
        set
        {
            m_musicVolume = value = Mathf.Clamp01(value);

            foreach(MusicSlot music in m_musics)
            {
                if (music.source != null)
                {
                    music.source.volume = value;
                }
            }
       }
    }

    public float SFXVolume
    {
        get
        {
            return m_sfxVolume;
        }
        set
        {
            m_sfxVolume = Mathf.Clamp01(value);
        }
    }

    public void PlaySound(E_Sound sound)
    {
        int soundIndex = (int)sound;
        if (m_sounds[soundIndex].clips == null || m_sounds[soundIndex].clips.Count == 0)
        {
            Debug.LogWarning("AudioClips empty: " + sound);
            return;
        }

        PlaySfx(m_sounds[soundIndex].clips[Random.Range(0, m_sounds[soundIndex].clips.Count)]);
    }

    public void PlaySfx(AudioClip audioClip)
    {
        AudioSource.PlayClipAtPoint(audioClip, GameManager.Instance.m_mainCamera.transform.position, m_sfxVolume);
    }

    public void PlayMusic(E_Music requestedMusic)
    {
        if (m_currentMusic == requestedMusic)
        {
            return;
        }

        AudioSource fadeOutMusic = null;
        AudioSource fadeInMusic = null;

        for (var index = 0; index < m_musics.Length; ++index)
        {
            if (m_musics[index].tag == m_currentMusic)
            {
                if (fadeOutMusic != null)
                {
                    Debug.LogWarning("Duplicate tag found (fadeOutMusic): " + m_currentMusic);
                }

                fadeOutMusic = m_musics[index].source;

                if (m_musics[index].source != null && m_musics[index].restorePosition)
                {
                    m_musics[index].lastPosition = Mathf.Max(m_musics[index].source.time - m_timeFadeInOut, 0.0f);
                }
            }
            else
            if (m_musics[index].tag == requestedMusic)
            {
                if (fadeInMusic != null)
                {
                    Debug.LogWarning("Duplicate tag found (fadeInMusic): " + requestedMusic);
                }

                fadeInMusic = m_musics[index].source;

                if (m_musics[index].source != null && m_musics[index].restorePosition)
                {
                    m_musics[index].source.time = m_musics[index].lastPosition;
                }
            }
        }

        m_currentMusic = requestedMusic;

        ChangeMusic(fadeOutMusic, fadeInMusic);
    }

    public void ChangeMusic(AudioSource fadeOut, AudioSource fadeIn) 
    {
        m_audioFadeInOut.StartFadeOutIn(fadeOut, m_timeFadeInOut, fadeIn, m_timeFadeInOut);
        MusicVolume = m_musicVolume;
    }

    public E_Music CurrentMusic
    {
        get
        {
            return m_currentMusic;
        }
    }
}
