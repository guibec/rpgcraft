using System;
using UnityEngine;
using System.Collections;

/// <summary>
/// Move toward a definitive 2d position
/// Move toward a relative 2d position (given a target)
/// </summary>
public class Mover : MonoBehaviour
{
    public enum E_TargetType
    {
        Absolute,
        Relative,
    }

    public E_TargetType TargetType;

    // Absolute & Relative require Target
    public Vector2 Target;

    public float TimeToTarget;

    private Vector2 OriginPosition;
    private float m_currentTime;
    private bool Enabled = false;
    private InterpolationCompletedDelegate m_onCompleted;

	// Use this for initialization
	void Start () 
    {
	
	}

    public delegate void InterpolationCompletedDelegate();

    public void StartInterpolation( Vector2 target, float duration, InterpolationCompletedDelegate callback )
    {
        m_currentTime = 0;
        TimeToTarget = duration;
        Target = target;
        OriginPosition = gameObject.transform.position;
        TargetType = E_TargetType.Absolute;
        m_onCompleted = callback;
        Enabled = true;
    }
	
	// Update is called once per frame
	void Update () 
    {
	    if (Enabled)
	    {
	        m_currentTime += TimeManager.Dt;
	        m_currentTime = Math.Min(m_currentTime, TimeToTarget);

	        float ratio = m_currentTime/TimeToTarget;

	        Vector2 interpolatedPos = (1.0f - ratio)*OriginPosition + ratio*Target;
	        gameObject.transform.position = interpolatedPos;
	    }

	    if (m_currentTime >= TimeToTarget)
	    {
	        OnCompleted();
	    }
	}

    private void OnCompleted()
    {
        Enabled = false;
        if (m_onCompleted != null)
            m_onCompleted();
    }
}
