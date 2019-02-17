using UnityEngine;
using System;
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

    private bool forced;

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
    private readonly Dictionary<Type, State> m_states = new Dictionary<Type, State>(5);

    protected void RegisterState(Type state)
    {
        State stateInstance = Activator.CreateInstance(state, this) as State;
        m_states[state] = stateInstance;
    }

    private void RegisterState(State stateInstance)
    {
        m_states.Add(stateInstance.GetType(), stateInstance);
    }

    public State FindStateByType(Type stateType)
    {
        foreach (KeyValuePair<Type, State> test in m_states)
        {
            if (stateType.IsAssignableFrom(test.Key))
                return test.Value;
        }

        return null;
    }

    public T FindStateByType<T>() where T : State
    {
        return FindStateByType(typeof(T)) as T;
    }

    protected StateMachine(MonoBehaviour mb)
    {
        Locked = false;
        MonoBehaviour = mb;
    }

    public void Update()
    {
        if (NextState != null)
        {
            if (CurrentState != null)
            {
                PreviousState = CurrentState;
                PreviousState.Destructor();

                OnStateDestructor?.Invoke(this, PreviousState);
            }

            CurrentState = NextState;

            NextState = null;
            CurrentState.Constructor();

            OnStateConstructor?.Invoke(this, CurrentState);

            forced = false;
        }

        CurrentState?.Update();
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

        if (nextState == null || (NextState != null))
            return;

        NextState = nextState;

        OnStateChanged?.Invoke(this, nextState);
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

        NextState = nextState;
        forced = true;

        OnStateForcedChanged?.Invoke(this, nextState);
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

        OnStateReload?.Invoke(this, CurrentState);
    }

    /// <summary>
    /// Return true is the current state is of the type
    /// </summary>
    public bool IsInState(Type type)
    {
        if (type == null)
            return false;

        return type.IsAssignableFrom(CurrentState.GetType());
    }

    public bool IsInState<T>() where T : State
    {
        return IsInState(typeof(T));
    }
}
