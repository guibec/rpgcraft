using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample18_TextField : MonoBehaviour 
{
    // The text field attribute change how a string is displayed.
    // In area mode, it grows with new lines.
    [Inspect, TextField(TextFieldType.Area)]
    public string myArea;

    // In File mode, the text field is turned into a file picker.
    // The string stores the path to the file.
    [Inspect, TextField(TextFieldType.File)]
    public string myFile;

    // Similar to the file one, this one invoke a folder picker.
    [Inspect, TextField(TextFieldType.Folder)]
    public string myFolder;

    // This one is similar to password field, only showing *****
    [Inspect, TextField(TextFieldType.Password)]
    public string myPassword;

    // This is a drop down list of the available tag.
    // Unity handles tag as string, but don't expose a tag selector.
    [Inspect, TextField(TextFieldType.Tag)]
    public string myTag;
}
