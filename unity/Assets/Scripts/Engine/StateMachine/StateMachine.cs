using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;

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

    public event StateEventHandler OnStateChanged;
    public event StateEventHandler OnStateConstructor;
    public event StateEventHandler OnStateDestructor;

    /// <summary>
    /// Registered states
    /// </summary>
    private readonly Dictionary<Type, State> m_states = new Dictionary<Type, State>(5);

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

    protected StateMachine(MonoBehaviour mb, Type[] states, Type initialState=null)
    {
        MonoBehaviour = mb;

        foreach (Type state in states)
        {
            State stateInstance = System.Activator.CreateInstance(state, this) as State;
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
            Assert.Fail("State machine does not have any state.");
        }
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
        }

        if (CurrentState != null)
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
    /// Return true is the current state is of the type
    /// </summary>
    public bool IsInState(Type type)
    {
        if (type == null)
            return false;

        if (CurrentState == null || CurrentState.GetType() == null)
        {
            Debug.Log("Break here!");
        }

        return type.IsAssignableFrom(CurrentState.GetType());
    }

    public bool IsInState<T>() where T : State
    {
        return IsInState(typeof(T));
    }
}
