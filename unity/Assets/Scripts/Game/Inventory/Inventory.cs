using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public class Inventory
{
    private Inventory_Data m_inventoryData;
    public Inventory_Data InventoryData
    {
        get
        {
            return m_inventoryData;
        }
        set
        {
            m_inventoryData = value;
            OnChanged();
        }
    }

    public readonly int MAX_ITEMS_PER_SLOT = 99;

    public delegate void InventoryChangedEventHandler(object sender, EventArgs e);
    public event InventoryChangedEventHandler Changed;

    public Inventory(Player player_)
    {
        Inventory_Data inventoryData;

        inventoryData.m_itemSlots = new ItemCount[10];
        for (int i = 0; i < inventoryData.m_itemSlots.Length; ++i)
            inventoryData.m_itemSlots[i] = new ItemCount();

        // default inventory
        inventoryData.m_itemSlots[0].Count = 1;
        inventoryData.m_itemSlots[0].Item = EItem.PickAxe;

        inventoryData.m_itemSlots[1].Count = 1;
        inventoryData.m_itemSlots[1].Item = EItem.Sword;

        inventoryData.m_itemSlots[2].Count = 1;
        inventoryData.m_itemSlots[2].Item = EItem.Copper_Axe;

        inventoryData.m_itemSlots[3].Count = 10;
        inventoryData.m_itemSlots[3].Item = EItem.Bomb;

        inventoryData.m_itemSlots[4].Count = 10;
        inventoryData.m_itemSlots[4].Item = EItem.Arrow;

        InventoryData = inventoryData;
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

        InventoryData.m_itemSlots[index].Item = item;
        InventoryData.m_itemSlots[index].Count++;

        //UnityEngine.Debug.Log("Adding item " + item + " to slot " + index + " with count of " + m_itemSlots[index].Count);
        OnChanged();

        return true;
    }

    public bool Use(int slotIndex)
    {
        if (InventoryData.m_itemSlots[slotIndex].Count >= 1)
        {
            InventoryData.m_itemSlots[slotIndex].Count--;
            if (InventoryData.m_itemSlots[slotIndex].Count == 0)
            {
                InventoryData.m_itemSlots[slotIndex].Item = EItem.None;
            }

            OnChanged();
            return true;
        }

        return false;
    }

    private int FindBestSlotFor(EItem item)
    {
        // look for existing slot
        for (int i = 0; i < InventoryData.m_itemSlots.Length; ++i)
        {
            if (InventoryData.m_itemSlots[i].Item == item && InventoryData.m_itemSlots[i].Count < MAX_ITEMS_PER_SLOT)
                return i;
        }

        // try to find an empty slot
        for (int i = 0; i < InventoryData.m_itemSlots.Length; ++i)
        {
            if (InventoryData.m_itemSlots[i].Item == EItem.None)
                return i;
        }

        return -1;
    }

    private void OnChanged()
    {
        Changed?.Invoke(this, EventArgs.Empty);
    }

    public ItemCount GetSlotInformation(int index_)
    {
        return InventoryData.m_itemSlots[index_];
    }
}
