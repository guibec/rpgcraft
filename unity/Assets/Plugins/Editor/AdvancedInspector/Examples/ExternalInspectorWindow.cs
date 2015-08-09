using UnityEngine;
using UnityEditor;

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

using AdvancedInspector;

public class ExternalInspectorWindow : EditorWindow
{
    private ExternalEditor editor;

    private GameObject go;
    private Component component;

    [MenuItem("Window/Advanced Insector Example")]
    public static void Init()
    {
        ExternalInspectorWindow window = EditorWindow.GetWindow<ExternalInspectorWindow>();
        window.wantsMouseMove = true;
        window.editor = ExternalEditor.CreateInstance<ExternalEditor>();

        window.editor.DraggableSeparator = false;
        window.editor.DivisionSeparator = 150;
    }

    private void OnSelectionChange()
    {
        go = Selection.activeGameObject;
        Repaint();
    }

    private void OnGUI()
    {
        if (go == null)
        {
            GUILayout.Label("Select a GameObject...");
            component = null;
            editor.Instances = new object[0];
        }
        else
        {
            Component[] components = go.GetComponents(typeof(Component));
            GUIContent[] contents = new GUIContent[components.Length + 1];
            contents[0] = new GUIContent("None");
            int index = -1;
            for (int i = 0; i < components.Length; i++)
            {
                contents[i + 1] = new GUIContent(components[i].GetType().Name);
                if (components[i] == component)
                    index = i + 1;
            }

            EditorGUI.BeginChangeCheck();
            index = EditorGUILayout.Popup(new GUIContent("Select a component: "), index, contents);
            if (EditorGUI.EndChangeCheck())
            {
                if (index == 0)
                {
                    component = null;
                    editor.Instances = new object[0];
                }
                else
                {
                    component = components[index - 1];
                    editor.Instances = new object[] { component };
                }
            }
        }

        if (editor.Draw(new Rect(0, 16, position.width, position.height - 16)))
            Repaint();
    }
}
