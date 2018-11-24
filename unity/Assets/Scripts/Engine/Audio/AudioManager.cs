using UnityEngine;
using System.Collections;
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
    public AudioSource m_worldMapMusic;
    public AudioSource m_battleMusic;
    public AudioSource m_bossBattleMusic;
    // Next person to add a music here makes a generic system!

    // Like this for now, data driven later on
    public List<AudioClip> m_digAudio;
    public List<AudioClip> m_cutAudio;

    public List<AudioClip> m_hitAudio;

    private E_Music m_currentMusic = E_Music.None;
    private float m_lastWorldMapMusicTime = 0;

    private AudioFadeInOut m_audioFadeInOut;
    protected override void Awake()
    {
        m_audioFadeInOut = gameObject.GetComponent<AudioFadeInOut>();
        DebugUtils.Assert(m_audioFadeInOut != null);

        base.Awake();
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

    static private void PlayAudioClip(List<AudioClip> possibilities)
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

        AudioSource.PlayClipAtPoint(possibilities[indexToPlay], GameManager.Instance.m_mainCamera.transform.position);
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
                m_lastWorldMapMusicTime = m_worldMapMusic.time; // technically incorrect due to the fadeout
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
        m_audioFadeInOut.StartFadeOutIn(fadeOut, 0.5f, fadeIn, 0.5f);
    }

    public E_Music CurrentMusic
    {
        get
        {
            return m_currentMusic;
        }
    }
}
