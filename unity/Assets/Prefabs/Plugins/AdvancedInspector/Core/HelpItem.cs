using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Used to display an help box on an item.
    /// </summary>
    public class HelpItem : IEquatable<HelpItem>
    {
        private HelpType type;

        /// <summary>
        /// Type of the help, changes the icons.
        /// </summary>
        public HelpType Type
        {
            get { return type; }
            set { type = value; }
        }

        private string message;

        /// <summary>
        /// Message displayed in the help box.
        /// </summary>
        public string Message
        {
            get { return message; }
            set { message = value; }
        }

        private HelpPosition position;

        /// <summary>
        /// Is the help box after or before the item?
        /// </summary>
        public HelpPosition Position
        {
            get { return position; }
            set { position = value; }
        }

        public HelpItem(HelpType type, string message)
            : this(type, message, HelpPosition.After) { }

        public HelpItem(HelpType type, string message, HelpPosition position)
        {
            this.type = type;
            this.message = message;
            this.position = position;
        }

        public bool Equals(HelpItem help)
        {
            return help.type == type && help.position == position && help.message == message;
        }

        public override bool Equals(object obj)
        {
            HelpItem help = obj as HelpItem;
            if (help == null)
                return false;

            return help.type == type && help.position == position && help.message == message;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }
}
