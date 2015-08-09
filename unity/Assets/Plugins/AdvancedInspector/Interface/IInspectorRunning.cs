using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// When implementing this, gives the object ability to draw the header and footer of the Inspector's space.
    /// </summary>
    public interface IInspectorRunning
    {
        /// <summary>
        /// Draw at the top of the inspector, in this order;
        /// - This
        /// - Class Helpbox
        /// - Tabs
        /// rest of the fields
        /// </summary>
        void OnHeaderGUI();

        /// <summary>
        /// Draw at the bottom of the inspector, in this order;
        /// - Helpbox
        /// - This
        /// </summary>
        void OnFooterGUI();
    }
}