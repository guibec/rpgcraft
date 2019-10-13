# rpgcraft
RPGCraft - Minecraft / Terraria / RPGMaker mashup

# Setup
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
  
## Map Generation
- TODO

## Tile Rendering
- TODO

## Collision System
- TODO

## Game Object Management
- TODO

## Inventory System
- TODO
