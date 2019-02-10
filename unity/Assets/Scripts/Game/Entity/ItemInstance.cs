using UnityEngine;
using System.Collections;

/// <summary>
/// ItemInstance is a concrete Item spawned in the map. It contains the actual Item information + whatever logic is required for the item (collision, rendering, sfx, etc.)
/// </summary>
public class ItemInstance : Entity
{
    public ETile Item { get; private set; }

    // Use this for initialization
    protected override void OnStart ()
    {
        base.OnStart();
        ItemManager.Instance.Register(this);
    }

    public override void OnTouch(Entity other)
    {
        base.OnTouch(other);
    }

    public void PickedUp(Entity source)
    {
        RequestDestroy();
    }

    public void SetType(ETile itemType)
    {
        Item = itemType;
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();
    }

    public static Color32 GetColor32ForItem(ETile item)
    {
        if (item == ETile.Gel)
            return new Color32(9, 129, 248, 255);
        else
            return new Color32(255, 255, 255, 255);
    }

    public static Color GetColorForItem(ETile item)
    {
        return GetColor32ForItem(item); // Cast operation is overloaded: http://answers.unity3d.com/questions/634208/convert-color32-to-color.html
    }

    // http://docs.unity3d.com/ScriptReference/MonoBehaviour.OnDestroy.html
    // OnDestroy will only be called on game objects that have previously been active
    protected override void OnEntityDestroy()
    {
        ItemManager.Instance.Unregister(this);

        base.OnEntityDestroy();
    }
}
