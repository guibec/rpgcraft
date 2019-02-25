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
    public MonoBehaviour MonoBehaviour { get; }

    /// <summary>
    /// Current state.
    /// </summary>
    public State CurrentState { get; private set; }

    /// <summary>
    /// Fired when the state is changed
    /// </summary>
    public event StateEventHandler OnStateChanged;

    /// <summary>
    /// Fired after the constructor of the new state has been called
    /// </summary>
    public event StateEventHandler OnStateConstructor;

    /// <summary>
    /// Fired after the destructor of the new state has been called
    /// </summary>
    public event StateEventHandler OnStateDestructor;

    /// <summary>
    /// Registered states
    /// </summary>
    private readonly Dictionary<Type, State> m_states = new Dictionary<Type, State>(5);

    protected StateMachine(MonoBehaviour mb, Type[] states, Type initialState = null)
    {
        MonoBehaviour = mb;

        foreach (Type state in states)
        {
            State stateInstance = Activator.CreateInstance(state, this) as State;
            m_states[state] = stateInstance;
        }

        // Set the initial state
        if (initialState != null)
        {
            SetInitialState(initialState);
        }
        else if (states.Length > 0)
        {
            SetInitialState(states[0]);
        }
        else
        {
            DebugUtils.Assert(false);
        }
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

    public void Update()
    {
        DebugUtils.Assert(CurrentState != null);
        CurrentState.Update();
    }

    /// <summary>
    /// Set the initial state of the state machine.
    /// This method will do nothing if called again after being initialized
    /// </summary>
    /// <param name="state">The starting state</param>
    private void SetInitialState(Type state)
    {
        State initialState = FindStateByType(state);
        SetInitialState(initialState);
    }

    private void SetInitialState(State initialState)
    {
        SwitchState(initialState);
    }

    /// <summary>
    /// Switch the state on the next frame.
    /// If null, will remove a previous request.
    /// </summary>
    private void SwitchState(State nextState)
    {
        if (nextState == null)
            return;

        // This check is only needed for the initial state
        if (CurrentState != null)
        {
            CurrentState.Destructor();
            OnStateDestructor?.Invoke(this, CurrentState);
        }

        CurrentState = nextState;
        CurrentState.Constructor();
        OnStateConstructor?.Invoke(this, CurrentState);

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
