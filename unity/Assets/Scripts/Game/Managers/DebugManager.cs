using System;
using UnityEngine;
using System.Collections.Generic;
using UnityEngine.Profiling;
using UnityEngine.SceneManagement;

public class DebugManager : MonoSingleton<DebugManager>
{
    private bool m_displayDebug = false;
    private Texture m_biomeTexture;

    protected override void OnUpdate()
    {
        base.OnUpdate();

        if (Input.GetKeyDown(KeyCode.Escape))
        {
            m_displayDebug = !m_displayDebug;
        }
    }

    // Make the contents of the window.
    void DoDebugWindow(int windowID)
    {
        // TODO: button layout / placement should be automated
        GUI.Button(new Rect(10, 30, 140, 20), "Options...");
        GUI.Button(new Rect(10, 60, 140, 20), "Generate new level...");
        GUI.Button(new Rect(10, 90, 140, 20), "Save level...");
        GUI.Button(new Rect(10, 120, 140, 20), "Load level...");

        GUI.Button(new Rect(10, 180, 140, 20), "Back to game");

        m_biomeTexture = GameManager.Instance.WorldMap.BiomeTexture;

        if (m_biomeTexture != null)
        {
            GUI.DrawTexture(new Rect(250, 50, 256, 256), m_biomeTexture, ScaleMode.ScaleToFit, true);
        }
    }

    void OnGUI()
    {
        // Make a toggle button for hiding and showing the window
        m_displayDebug = GUI.Toggle(new Rect(Screen.width - 100, 10, 100, 20), m_displayDebug, "Game Menu");

        // Make sure we only call GUI.Window if doWindow0 is true.
        if (m_displayDebug)
        {
            GUI.Window(0, new Rect(Screen.width / 8, Screen.height / 8, Screen.width * 6 / 8, Screen.height * 6 / 8), DoDebugWindow, "Game menu");
        }

        // Next step, draw the biome map here:
        // using possibly: https://docs.unity3d.com/ScriptReference/GUI.DrawTexture.html
    }
}