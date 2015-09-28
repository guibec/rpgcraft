using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// Basic state machine.
/// Generally used for object that has multiple related or unrelated states.
/// </summary>
public abstract class StateMachine
{
    public delegate void StateEventHandler(object sender, State state);

    /// <summary>
    /// Owner of this StateMachine.
    /// </summary>
    public MonoBehaviour MonoBehaviour { get; private set; }

    /// <summary>
    /// The older destroyed state that was playing previous to the current state.
    /// </summary>
    public State PreviousState { get; private set; }

    /// <summary>
    /// Currently updated state.
    /// </summary>
    public State CurrentState { get; private set; }

    /// <summary>
    /// Next State.
    /// This will be this machine current state next frame.
    /// </summary>
    public State NextState { get; private set; }

    private bool forced = false;

    /// <summary>
    /// Prevent the machine from switching state.
    /// </summary>
    public bool Locked { get; set; }

    public event StateEventHandler OnStateChanged;
    public event StateEventHandler OnStateForcedChanged;
    public event StateEventHandler OnStateConstructor;
    public event StateEventHandler OnStateDestructor;
    public event StateEventHandler OnStateReload;

    /// <summary>
    /// Registered states
    /// </summary>
    private Dictionary<Type, State> m_states = new Dictionary<Type, State>(5);

    protected void RegisterState(Type state)
    {
        State stateInstance = System.Activator.CreateInstance(state, this) as State;
        m_states[state] = stateInstance;
    }

    private void RegisterState(State stateInstance)
    {
        m_states.Add(stateInstance.GetType(), stateInstance);
    }

    public State FindStateByType(Type stateType)
    {
        State state;
        return m_states.TryGetValue(stateType, out state) ? state : null;
    }

    public T FindStateByType<T>() where T : State
    {
        return FindStateByType(typeof(T)) as T;
    }

    protected StateMachine(MonoBehaviour mb)
    {
        Locked = false;
        this.MonoBehaviour = mb;
    }

    public void Update()
    {
        if (NextState != null)
        {
            if (CurrentState != null)
            {
                PreviousState = CurrentState;
                PreviousState.Destructor();

                if (OnStateDestructor != null)
                    OnStateDestructor(this, PreviousState);
            }

            CurrentState = NextState;

            NextState = null;
            CurrentState.Constructor();

            if (OnStateConstructor != null)
                OnStateConstructor(this, CurrentState);

            forced = false;
        }

        if (CurrentState != null)
            CurrentState.Update();
    }

    public void SetInitialState(Type state)
    {
        State initialState = FindStateByType(state);
        SetInitialState(initialState);
    }

    private void SetInitialState(State initialState)
    {
        CurrentState = null;
        PreviousState = null;
        NextState = initialState;
    }

    /// <summary>
    /// Switch the state on the next frame.
    /// If null, will remove a previous request.
    /// </summary>
    private void SwitchState(State nextState)
    {
        if (forced || Locked)
            return;

        if (nextState == null || (this.NextState != null))
            return;

        this.NextState = nextState;

        if (OnStateChanged != null)
            OnStateChanged(this, nextState);
    }

    public void SwitchState(Type state)
    {
        State nextState = FindStateByType(state);
        SwitchState(nextState);
    }

    public void SwitchState<T>() where T : State
    {
        SwitchState(typeof (T));
    }

    /// <summary>
    /// Forces the state to change, bypassing priorities.
    /// Any other following request will be ignored. Use with care.
    /// </summary>
    public void ForceSwitchState(State nextState)
    {
        if (forced || Locked)
            return;

        this.NextState = nextState;
        forced = true;

        if (OnStateForcedChanged != null)
            OnStateForcedChanged(this, nextState);
    }

    /// <summary>
    /// Force the current state to be reloaded.
    /// Simply call the destructor and recall the constructor.
    /// </summary>
    public void ReloadState()
    {
        if (CurrentState == null)
            return;

        CurrentState.Destructor();
        CurrentState.Constructor();

        if (OnStateReload != null)
            OnStateReload(this, CurrentState);
    }

    /// <summary>
    /// Return true is the current state is of the type
    /// </summary>
    public bool IsInState(Type type)
    {
        if (type == null)
            return false;

        return (CurrentState != null && type.IsInstanceOfType(CurrentState));
    }

    public bool IsInState<T>() where T : State
    {
        return IsInState(typeof(T));
    }
}
