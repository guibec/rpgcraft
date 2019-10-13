# rpgcraft
RPGCraft - Minecraft / Terraria / RPGMaker mashup

# Setup
- Install Visual Studio 2017 from https://visualstudio.microsoft.com/downloads/
  - Make sure to install C# and C++ support.
  - Note: Visual Studio 2019 hasn't been tested but will probably work.
- Install Unity 2018.3.7f1 from https://unity3d.com/get-unity/download/archive
  - Install the platforms that you want to test on.
  - Only PC has been tested.

## Run the Game
- Open the Scene `Assets/Scenes/main.unity`.
- Click Play.

## Test the Main Screen
- Open the Scene `Assets/Scenes/title.unity`
- Click Play

## Test the UI
- Open the Scene `Assets/Scene/ui.unity`

# Architecture
- All singletons are Unity objects and are named `__` in the Scene.
  - They are marked as DontDestroyOnLoad and thus survive between scenes reload.
- All Unity classes are overloaded:
  - 
- JSON.Net (Newtonsoft) is used for JSON to C# Structure serialization and vice-versa.
  - Found many quirks with built-in Unity JSON parsing.
  - JSON.Net is well supported.
  - JSON.Net provides auto-serialization, making serialization code trivial to write.
  - https://www.newtonsoft.com/json
  
## GameManager
- Responsible for the state of the game. Drives everything else.

## AudioManager
- Contains the music and SFX asset references.
- Contains API to Play and Stop audio.

# CollisionManager
- 
# DebugManager

# DataManager
- Loads all game data, this includes:
  - `enemiesInfo.json`: Loots, what enemies give when they are killed.
  - `tilesInfo.json`: Describe tiles (texture, transformation, properties such as speed, passable, etc.)
- Game data are all in json format
- No custom editor for json data (yet).

# EntityManager
- Responsible

# InputManager


# ItemManager

# RandomManager

# SpawnManager

# TimeManager

## Tile Rendering
- TODO

## Collision System
- TODO

## Game Object Management
- TODO

## Inventory System
- TODO
