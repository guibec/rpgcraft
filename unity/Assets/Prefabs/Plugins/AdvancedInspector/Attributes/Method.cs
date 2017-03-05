using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Used when inspected a method, gives control over how it is displayed or handled.
    /// If the undo message is not an empty string, the inspector will attempt to record a undo stack of the modified self-object.
    /// The Undo Message is not used when the method is in invoke mode.
    /// </summary>
    [AttributeUsage(AttributeTargets.Method)]
    public class MethodAttribute : Attribute
    {
        private MethodDisplay display = MethodDisplay.Button;

        public MethodDisplay Display
        {
            get { return display; }
            set { display = value; }
        }

        private string undoMessageOnClick = "";

        public string UndoMessageOnClick
        {
            get { return undoMessageOnClick; }
            set { undoMessageOnClick = value; }
        }

        private bool isCoroutine = false;

        public bool IsCoroutine
        {
            get { return isCoroutine; }
            set { isCoroutine = value; }
        }

        public MethodAttribute() { }

        public MethodAttribute(MethodDisplay display)
        {
            this.display = display;
        }

        public MethodAttribute(bool isCoroutine)
            : this (isCoroutine, "")
        { }

        public MethodAttribute(string undoMessageOnClick)
            : this(false, undoMessageOnClick)
        { }

        public MethodAttribute(bool isCoroutine, string undoMessageOnClick)
        {
            display = MethodDisplay.Button;
            this.undoMessageOnClick = undoMessageOnClick;
            this.isCoroutine = isCoroutine;
        }
    }
}