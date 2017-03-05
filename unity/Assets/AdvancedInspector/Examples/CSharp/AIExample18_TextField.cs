using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample18_TextField : MonoBehaviour
    {
        // The text field attribute change how a string is displayed.
        // In area mode, it grows with new lines.
        [TextField(TextFieldType.Area)]
        public string myArea;

        // In File mode, the text field is turned into a file picker.
        // The string stores the path to the file.
        [TextField(TextFieldType.File)]
        public string myFile;

        // Similar to the file one, this one invoke a folder picker.
        [TextField(TextFieldType.Folder)]
        public string myFolder;

        // This one is similar to password field, only showing *****
        [TextField(TextFieldType.Password)]
        public string myPassword;

        // This is a drop down list of the available tag.
        // Unity handles tag as string, but don't expose a tag selector.
        [TextField(TextFieldType.Tag)]
        public string myTag;
    }
}