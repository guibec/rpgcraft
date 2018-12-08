using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public class Inventory
{
    public readonly int MAX_ITEMS_PER_SLOT = 99;

    public delegate void InventoryChangedEventHandler(object sender, EventArgs e);
    public event InventoryChangedEventHandler Changed;

    public Inventory(Player player_)
    {
        Owner = player_;

        m_itemSlots = new ItemCount[10];
        for (int i = 0; i < m_itemSlots.Length; ++i)
            m_itemSlots[i] = new ItemCount();

        // default inventory
        m_itemSlots[0].Count = 1;
        m_itemSlots[0].Item = EItem.PickAxe;

        m_itemSlots[1].Count = 1;
        m_itemSlots[1].Item = EItem.Sword;

        m_itemSlots[2].Count = 1;
        m_itemSlots[2].Item = EItem.Copper_Axe;

        m_itemSlots[3].Count = 10;
        m_itemSlots[3].Item = EItem.Bomb;
    }

    public void Start()
    {
        OnChanged();
    }

    public bool Carry(EItem item)
    {
        int index = FindBestSlotFor(item);

        if (index == -1)
            return false;

        m_itemSlots[index].Item = item;
        m_itemSlots[index].Count++;

        UnityEngine.Debug.Log("Adding item " + item + " to slot " + index + " with count of " + m_itemSlots[index].Count);
        OnChanged();

        return true;
    }

    public bool Use(int slotIndex)
    {
        if (m_itemSlots[slotIndex].Count >= 1)
        {
            m_itemSlots[slotIndex].Count--;
            if (m_itemSlots[slotIndex].Count == 0)
            {
                m_itemSlots[slotIndex].Item = EItem.None;
            }

            OnChanged();
            return true;
        }

        return false;
    }

    private int FindBestSlotFor(EItem item)
    {
        // look for existing slot
        for (int i = 0; i < m_itemSlots.Length; ++i)
        {
            if (m_itemSlots[i].Item == item && m_itemSlots[i].Count < MAX_ITEMS_PER_SLOT)
                return i;
        }

        // try to find an empty slot
        for (int i = 0; i < m_itemSlots.Length; ++i)
        {
            if (m_itemSlots[i].Item == EItem.None)
                return i;
        }

        return -1;
    }

    private void OnChanged()
    {
        if (Changed != null)
            Changed(this, EventArgs.Empty);
    }

    public ItemCount GetSlotInformation(int index_)
    {
        return m_itemSlots[index_];
    }

    /// <summary>
    /// All items slot
    /// </summary>
    private ItemCount[] m_itemSlots;

    /// <summary>
    /// Amount of gold the player has
    /// </summary>
    public int Gold { get; set; }
    public Player Owner { get; private set; }
}
