using System;

/// <summary>
/// Derive each state of your machine from this.
/// </summary>
public abstract class State
{
    /// <summary>
    /// Owner of this state, even if this state is not currently referenced by the machine.
    /// </summary>
    public StateMachine FSM { get; private set; }

    protected State(StateMachine machine)
    {
        this.FSM = machine;
    }

    protected void SwitchState(Type newState)
    {
        FSM.SwitchState(newState);
    }

    protected void SwitchState<T>() where T : State
    {
        FSM.SwitchState<T>();
    }

    protected bool IsInState(Type type)
    {
        return FSM.IsInState(type);
    }

    protected bool IsInState<T>() where T : State
    {
        return FSM.IsInState<T>();
    }

    /// <summary>
    /// Called when a state is becoming active.
    /// </summary>
    public virtual void Constructor() { }

    /// <summary>
    /// Called when a state is no more active.
    /// </summary>
    public virtual void Destructor() { }

    /// <summary>
    /// Called each frame to update the state internal logic.
    /// </summary>
    public virtual void Update() { }
}