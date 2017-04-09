using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DebugWindow : MonoBehaviour {
    Rect windowRect = new Rect(20, 20, 300, 400);
    int windowId = 1;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.F1))
            Visible = !Visible;
    }

    void OnGUI()
    {
        if (Visible)
        {
            windowRect = GUI.Window(windowId, windowRect, UpdateGUI, "");
        }
    }

    public bool Visible { get; set; }

    void UpdateGUI(int windowId)
    {
        GUILayout.BeginArea(new Rect(3.0f, 0, windowRect.width, windowRect.height));

        GUILayout.Space(2);
        GUILayout.Box("Debug Window");

        if (GUILayout.Button("Close"))
        {
            Visible = false;
        }

        GUILayout.EndArea();

        GUI.DragWindow();
    }
}
