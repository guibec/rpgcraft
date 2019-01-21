using System.IO;
using UnityEngine;
using Newtonsoft.Json;

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

    public string PathToCharacterSave
    {
        get
        {
            return Application.persistentDataPath + "main.chr";
        }
    }

    JsonSerializerSettings jsonSettings = new JsonSerializerSettings { ReferenceLoopHandling = ReferenceLoopHandling.Ignore };

    private void SaveCharacter()
    {
        Player.Save_Data toSave = GameManager.Instance.MainPlayer.Save();
        string serializedData = JsonConvert.SerializeObject(toSave, Formatting.Indented, jsonSettings);

        Debug.Log(string.Format("Saving: \"{0}\" to file {1}", serializedData, PathToCharacterSave));

        StreamWriter writer = new StreamWriter(PathToCharacterSave, false);
        writer.Write(serializedData);
        writer.Close();
    }
    
    private bool LoadCharacter()
    {
        string serializedData;
        using (StreamReader reader = new StreamReader(PathToCharacterSave))
        {
            serializedData = reader.ReadToEnd();
        }

        Debug.Log(string.Format("Loading: \"{0}\" from file {1}", serializedData, PathToCharacterSave));

        if (serializedData.Length == 0)
        {
            return false;
        }

        Player.Save_Data saveData = JsonConvert.DeserializeObject<Player.Save_Data>(serializedData, jsonSettings);
        GameManager.Instance.MainPlayer.Load(saveData);

        return true;
    }

    // Make the contents of the window.
    void DoDebugWindow(int windowID)
    {
        // TODO: button layout / placement should be automated
        GUI.Button(new Rect(10, 30, 200, 20), "Options...");

        if (GUI.Button(new Rect(10, 60, 200, 20), "Generate new level..."))
        {
            RegenerateLevel();
        }

        GUI.Button(new Rect(10, 90, 200, 20), "Save level...");
        GUI.Button(new Rect(10, 120, 200, 20), "Load level...");

        if (GUI.Button(new Rect(10, 150, 200, 20), "(Debug) Save character..."))
        {
            SaveCharacter();
        }

        if (GUI.Button(new Rect(10, 180, 200, 20), "(Debug) Load character..."))
        {
            LoadCharacter();
        }

        if (GUI.Button(new Rect(10, 240, 200, 20), "Back to game"))
        {
            m_displayDebug = false;
        }

        m_biomeTexture = GameManager.Instance.WorldMap.BiomeTexture;

        if (m_biomeTexture != null)
        {
            GUI.DrawTexture(new Rect(250, 50, 256, 256), m_biomeTexture, ScaleMode.ScaleToFit, true, 0.0f);
        }
    }

    void OnGUI()
    {
        // Make a toggle button for hiding and showing the window
        m_displayDebug = GUI.Toggle(new Rect(Screen.width - 100, 10, 100, 20), m_displayDebug, "Game Menu");

        // Make sure we only call GUI.Window if doWindow0 is true.
        if (m_displayDebug)
        {
            GUI.Window((int)DebugWindowsID.GameMenu, new Rect(Screen.width / 8, Screen.height / 8, Screen.width * 6 / 8, Screen.height * 6 / 8), DoDebugWindow, "Game menu");
        }

        // Next step, draw the biome map here:
        // using possibly: https://docs.unity3d.com/ScriptReference/GUI.DrawTexture.html
    }

    private void RegenerateLevel()
    {
        GameManager.Instance.RegenerateWorld();
    }
}
