using System.IO;
using UnityEngine;
using Newtonsoft.Json;

public class DebugManager : MonoSingleton<DebugManager>
{
    private bool m_displayDebug;
    private bool m_displayOptions;

    private Texture m_biomeTexture;

    public float m_defaultOffsetX = 10.0f;
    public float m_defaultOffsetY = 0.0f;
    public float m_defaultWidth = 200.0f;
    public float m_defaultHeight = 20.0f;
    public float m_spaceY = 30.0f;

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

    readonly JsonSerializerSettings jsonSettings = new JsonSerializerSettings { ReferenceLoopHandling = ReferenceLoopHandling.Ignore };

    private void SaveCharacter()
    {
        Player.Save_Data toSave = GameManager.Instance.MainPlayer.Save();
        string serializedData = JsonConvert.SerializeObject(toSave, Formatting.Indented, jsonSettings);

        Debug.Log($"Saving: \"{serializedData}\" to file {PathToCharacterSave}");

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

        Debug.Log($"Loading: \"{serializedData}\" from file {PathToCharacterSave}");

        if (serializedData.Length == 0)
        {
            return false;
        }

        Player.Save_Data saveData = JsonConvert.DeserializeObject<Player.Save_Data>(serializedData, jsonSettings);
        GameManager.Instance.MainPlayer.Load(saveData);

        return true;
    }

    private void ReloadConfiguration()
    {
        DataManager.Instance.Reload();
    }

    // Make the contents of the window.
    void DoDebugWindow(int windowID)
    {
        float offsetX = m_defaultOffsetX;
        float offsetY = m_defaultOffsetY;
   
        if (GUI.Button(new Rect(offsetX, offsetY+=m_spaceY, m_defaultWidth, m_defaultHeight), "Options..."))
        {
            m_displayOptions = !m_displayOptions;
        }
        
        if (m_displayOptions)
        {
            GUI.Label(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "[Music Volume]");
            float musicVolume = GUI.HorizontalSlider(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), AudioManager.Instance.GetMusicVolume(), 0.0f, 1.0f);

            GUI.Label(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "[SFX Volume]");
            float sfxVolume = GUI.HorizontalSlider(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), AudioManager.Instance.GetSFXVolume(), 0.0f, 1.0f);

            AudioManager.Instance.SetMusicVolume(musicVolume);
            AudioManager.Instance.SetSFXVolume(sfxVolume);
        }

        if (GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "Generate new level..."))
        {
            RegenerateLevel();
        }

        GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "Save level...");
        GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "Load level...");

        if (GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "(Debug) Save character..."))
        {
            SaveCharacter();
        }

        if (GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "(Debug) Load character..."))
        {
            LoadCharacter();
        }

        if (GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "(Debug) Reload configuration files"))
        {
            ReloadConfiguration();
        }

        if (GUI.Button(new Rect(offsetX, offsetY += m_spaceY, m_defaultWidth, m_defaultHeight), "Back to game"))
        {
            m_displayDebug = false;
        }

        m_biomeTexture = GameManager.Instance.WorldMap.BiomeTexture;

        if (m_biomeTexture != null)
        {
            GUI.DrawTexture(new Rect(offsetX + m_defaultWidth + 46, 50, 256, 256), m_biomeTexture, ScaleMode.ScaleToFit, true, 0.0f);
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

    public bool IsVisible()
    {
        return m_displayDebug;
    }
}
