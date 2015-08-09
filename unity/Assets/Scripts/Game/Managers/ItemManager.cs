using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class ItemManager : MonoSingleton<ItemManager>, IEnumerable
{
    private HashSet<ItemInstance> m_items = new HashSet<ItemInstance>();

    public void Register(ItemInstance _item)
    {
        m_items.Add(_item);
    }

    public void Unregister(ItemInstance _item)
    {
        m_items.Remove(_item);
    }

    public IEnumerator GetEnumerator()
    {
        return m_items.GetEnumerator();
    }
}
