using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(TextMesh), true)]
    public class TextMeshEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(TextMesh);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("text"), new TextFieldAttribute(TextFieldType.Area),
                new DescriptorAttribute("Text", "The text that is displayed.", "http://docs.unity3d.com/ScriptReference/TextMesh-text.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("font"),
                new DescriptorAttribute("Font", "The Font used.", "http://docs.unity3d.com/ScriptReference/TextMesh-font.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("color"),
                new DescriptorAttribute("Color", "The color used to render the text.", "http://docs.unity3d.com/ScriptReference/TextMesh-color.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("fontSize"),
                new DescriptorAttribute("Font Size", "The font size to use (for dynamic fonts).", "http://docs.unity3d.com/ScriptReference/TextMesh-fontSize.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("characterSize"),
                new DescriptorAttribute("Character Size", "The size of each character (This scales the whole text).", "http://docs.unity3d.com/ScriptReference/TextMesh-characterSize.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("tabSize"),
                new DescriptorAttribute("Tab Size", "How much space will be inserted for a tab '\t' character. This is a multiplum of the 'spacebar' character offset.", "http://docs.unity3d.com/ScriptReference/TextMesh-tabSize.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("lineSpacing"),
                new DescriptorAttribute("Line Spacing", "How much space will be in-between lines of text.", "http://docs.unity3d.com/ScriptReference/TextMesh-lineSpacing.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("offsetZ"),
                new DescriptorAttribute("Z Offset", "How far should the text be offset from the transform.position.z when drawing.", "http://docs.unity3d.com/ScriptReference/TextMesh-offsetZ.html")));


            Fields.Add(new InspectorField(type, Instances, type.GetProperty("fontStyle"),
                new DescriptorAttribute("Font Style", "The font style to use (for dynamic fonts).", "http://docs.unity3d.com/ScriptReference/TextMesh-fontStyle.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("anchor"),
                new DescriptorAttribute("Anchor", "Which point of the text shares the position of the Transform.", "http://docs.unity3d.com/ScriptReference/TextMesh-anchor.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("alignment"),
                new DescriptorAttribute("Alignment", "How lines of text are aligned (Left, Right, Center).", "http://docs.unity3d.com/ScriptReference/TextMesh-alignment.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("richText"),
                new DescriptorAttribute("Rich Text", "Enable HTML-style tags for Text Formatting Markup.", "http://docs.unity3d.com/ScriptReference/TextMesh-richText.html")));
        }
    }
}
