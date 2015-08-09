using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Implements the method required to display an inspector preview. 
    /// </summary>
    public interface IPreview
    {
        /// <summary>
        /// This should return instance(s) of the following type;
        /// GameObject
        /// Mesh
        /// Material
        /// Texture
        /// Cubemap
        /// If return null or empty array, preview is turned off.
        /// </summary>
        UnityEngine.Object[] Preview { get; }
    }
}