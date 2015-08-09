using UnityEngine;
using System;
using System.Collections;

namespace AdvancedInspector
{
    /// <summary>
    /// Define an interface called when the Inspector has performed changes.
    /// The event works the other way around, as a way to notify the Inspector something changed and needs to be refreshed.
    /// </summary>
    public interface IDataChanged
    {
        /// <summary>
        /// Fired when the Inspector changed.
        /// </summary>
        void DataChanged();

        /// <summary>
        /// Should be fired internal by the object when the fields structure changed.
        /// Ex.: Added an object to a list.
        /// </summary>
        event GenericEventHandler OnDataChanged;
    }
}