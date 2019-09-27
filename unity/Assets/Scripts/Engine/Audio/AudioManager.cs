using UnityEngine;
using System.Collections.Generic;

public enum E_Music
{
    None,
    WorldMap,
    Battle,
    BossBattle,
}

[RequireComponent(typeof(AudioFadeInOut))]
public class AudioManager : MonoSingleton<AudioManager>
{
    public float m_musicVolume = 0.5f;
    public float m_sfxVolume = 1.0f;

    public float m_timeFadeInOut = 0.5f;

    public AudioSource m_worldMapMusic;
    public AudioSource m_battleMusic;
    public AudioSource m_bossBattleMusic;
    // Next person to add a music here makes a generic system!

    // Like this for now, data driven later on
    public List<AudioClip> m_digAudio;
    public List<AudioClip> m_cutAudio;

    public List<AudioClip> m_hitAudio;

    private E_Music m_currentMusic = E_Music.None;
    private float m_lastWorldMapMusicTime;

    private AudioFadeInOut m_audioFadeInOut;
    protected override void Awake()
    {
        m_audioFadeInOut = gameObject.GetComponent<AudioFadeInOut>();
        DebugUtils.Assert(m_audioFadeInOut != null);

        base.Awake();
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

            m_worldMapMusic.volume = value;
            m_battleMusic.volume = value;
            m_bossBattleMusic.volume = value;
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

    public void PlaySfx(AudioClip audioClip)
    {
        AudioSource.PlayClipAtPoint(audioClip, GameManager.Instance.m_mainCamera.transform.position, m_sfxVolume);
    }

    private void PlayAudioClip(List<AudioClip> possibilities)
    {
        int indexToPlay = 0;
        if (possibilities == null || possibilities.Count == 0)
        {
            return;
        }
        else if (possibilities.Count > 1)
        {
            indexToPlay = Random.Range(0, possibilities.Count);
        }

        PlaySfx(possibilities[indexToPlay]);
    }

    public void PlayMusic(E_Music requestedMusic)
    {
        if (m_currentMusic == requestedMusic)
        {
            return;
        }

        AudioSource fadeOutMusic = null;
        switch (m_currentMusic)
        {
            case E_Music.WorldMap:
                m_lastWorldMapMusicTime = Mathf.Max(m_worldMapMusic.time - m_timeFadeInOut, 0.0f);
                fadeOutMusic = m_worldMapMusic;
                break;
            case E_Music.Battle:
                fadeOutMusic = m_battleMusic;
                break;
            case E_Music.BossBattle:
                fadeOutMusic = m_bossBattleMusic;
                break;
            default:
                break;
        }

        m_currentMusic = requestedMusic;

        AudioSource fadeInMusic = null;
        switch (m_currentMusic)
        {
            case E_Music.WorldMap:
                m_worldMapMusic.time = m_lastWorldMapMusicTime;
                fadeInMusic = m_worldMapMusic;
                break;
            case E_Music.Battle:
                fadeInMusic = m_battleMusic;
                break;
            case E_Music.BossBattle:
                fadeInMusic = m_bossBattleMusic;
                break;
            default:
                Debug.Break();
                break;
        }

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
