using UnityEngine;
using System.Collections;

/// <summary>
/// ItemInstance is a concrete Item spawned in the map. It contains the actual Item information + whatever logic is required for the item (collision, rendering, sfx, etc.)
/// </summary>
public class ItemInstance : Entity
{
    public EItem Item { get; private set; }

	// Use this for initialization
	protected override void OnStart ()
	{
        base.OnStart();
	    ItemManager.Instance.Register(this);
	}

    public override void OnTouch(Entity other)
    {
        base.OnTouch(other);

        RequestDestroy();
    }

    public void SetType(EItem itemType)
    {
        Item = itemType;
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();
    }

    // http://docs.unity3d.com/ScriptReference/MonoBehaviour.OnDestroy.html
    // OnDestroy will only be called on game objects that have previously been active
    protected override void OnEntityDestroy()
    {
        ItemManager.Instance.Unregister(this);

        base.OnEntityDestroy();
    }
}
