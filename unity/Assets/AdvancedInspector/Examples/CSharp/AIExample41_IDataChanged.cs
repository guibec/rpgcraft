using System.Collections.Generic;
using UnityEngine;

namespace AdvancedInspector
{
    // The Inspector, by default, is lazy.
    // Which means that is it doesn't trap a change itself, it has no way to know a change actually occured.
    // For performance reason, the inspector is only refreshed when such changed is know.
    // Unity's default inspector is serialization bound, so it checks if something in the saved file change to refresh.
    // Advanced Inspector is able to inspect properties and other items not bound to serialization.
    // Therefor, if something changed outside the inspector's scope, you have to tell AI that a changed occured.
    // The IDataChanged interfaces allows for a two-way street communication.
    // Raising the OnDataChanged event tells the Inspector something changed - maybe by code - and the inspector should be refreshed.
    public class AIExample41_IDataChanged : MonoBehaviour, IDataChanged
    {
        [Inspect, Collection(0)]
        private List<GameObject> list = new List<GameObject>();

        public event GenericEventHandler OnDataChanged;

        public void DataChanged() { }

        [Inspect]
        private void AddItem()
        {
            for (int i = 0; i < 4; i++)
                list.Add(null);

            if (OnDataChanged != null)
                OnDataChanged();
        }
    }
}
