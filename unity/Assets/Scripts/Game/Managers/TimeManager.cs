using UnityEngine;
using System;

public class TimeManager : MonoSingleton<TimeManager> 
{
    protected override void OnUpdate()
    {
        if (m_lastTick == 0)
            m_lastTick = DateTime.Now.Ticks;

        long now = DateTime.Now.Ticks;
        //long deltaTick = now - m_lastTick;

        //m_dt = ((deltaTick) / TimeSpan.TicksPerMillisecond) / 1000.0f;
        m_dt = Time.deltaTime;

        // Minimal frame rate of 30 fps (for now...)
        //if (m_dt >= 0.033f)
        //  m_dt = 0.033f;

        if (m_currentTick != 0)
            m_lastTick = m_currentTick;

        m_currentTick = now;

        m_frameCount++;
        if ((m_frameCount % 10) == 0)
        {
            //UnityEngine.Debug.Log("Frame time is " + m_dt * 1000.0f + " ms - Unity frame time is " + Time.deltaTime * 1000.0f + " ms");
        }
    }

    public static float Now
    {
        get { return Time.time; }
    }

    public static float Dt
    {
        get
        {
            return Instance.m_dt;
        }
        set
        {
            Instance.m_dt = value;
        }
    }

    // dt info
    private long m_lastTick, m_currentTick;
    private float m_dt = 0.0f;
    private long m_frameCount = 0;
}
