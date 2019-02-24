using System;
using UnityEngine;

/// <summary>
/// Move toward a definitive 2d position
/// Move toward a relative 2d position (given a target)
/// </summary>
public class Mover : MonoBehaviourEx
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
    private bool Enabled;
    private InterpolationCompletedDelegate m_onCompleted;

    // Use this for initialization
    void Start () 
    {
    
    }

    public delegate void InterpolationCompletedDelegate();

    // Speed will depend on distance and duration
    public void StartInterpolation( Vector2 target, float duration, InterpolationCompletedDelegate callback=null )
    {
        m_currentTime = 0;
        TimeToTarget = duration;
        Target = target;
        OriginPosition = gameObject.transform.position;
        TargetType = E_TargetType.Absolute;
        m_onCompleted = callback;
        Enabled = true;
    }

    // Duration will depend on speed and distance
    public void StartInterpolationConstantSpeed( Vector2 target, float speed, InterpolationCompletedDelegate callback=null)
    {
        Vector2 origin = new Vector2(gameObject.transform.position.x, gameObject.transform.position.y);
        float distance = (origin - target).magnitude;

        // distance is too small, just go directly!
        if (distance <= 0.01)
        {
            StartInterpolation(target, 0.0f, callback);
        }
        else
        {
            float duration = distance / speed;
            StartInterpolation(target, duration, callback);
        }
        
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
        m_onCompleted?.Invoke();
    }
}
