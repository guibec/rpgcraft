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
  - `MonoBehaviorEx` replaces `MonoBehavior`
- `Scripts/Engine` is for general purpose code.
- `Scripts/Game` is for game specific code.
- JSON.Net (Newtonsoft) is used for JSON to C# Structure serialization and vice-versa.
  - Found many quirks with built-in Unity JSON parsing.
  - JSON.Net is well supported.
  - JSON.Net provides auto-serialization, making serialization code trivial to write.
  - https://www.newtonsoft.com/json
  
## World Generation
- World is generated with Chunk. Each Chunk represents a 64x64 tiles.
- Each Tile is drawn using 2 triangles, 6 vertices. Each Chunk thus has 24,576 vertices.
- Game Coordinate starts from (0,0). Each tile measure 1x1 in game coordinate.
- At initial world generation, the chunk where the player is is generated. Neighborhood chunks are always generated.
- Generating chunks is expensive, and a noticeable hitch is felt whenever a new chunk is generated.

## Entity / Collision
- Every game object in the game is a `Entity` and is tracked by the `EntityManager`.
-- For example, `Player` is an Entity. `Slime` is a `Enemy` which is a type of `Entity..
- The Unity collision system is not used to track collision.
-- This is because it is a physics based system, which was overkill.
- The `CollisionManager` takes care of associating each `Entity` with a `Chunk`. This is an optimization for collision to prevent checking for collision between entities that can't touch each other.
- Collisions detection is dynamic, it should be impossible for an object to tunnel through another object without colliding
-- Two collided objects will be replaced so that they don't collide. This is done through a recalage algorithm that pushes back objects.
- Some of the collision code handling the main player is in `GameManager`.
- The game will side-step the player if colliding on the side, to allow for better navigation, similar to Zelda 1.
- Animation code is custom and is done through the `EntityRender` class
-- EntityRender: Contains a list of animation (by names) and a sequence of frames for the animation. 
-- Enemies expect the following animations: `Left`, `Right`, `Up` and `Down`.
  
## GameManager
- Responsible for the state of the game. Drives everything else.

## AudioManager
- Contains the music and SFX asset references.
- Contains API to Play and Stop audio.

# CollisionManager
- APIs for detecting collision

# DebugManager
- General debugging functionalities.

# DataManager
- Loads all game data, this includes:
  - `enemiesInfo.json`: Loots, what enemies give when they are killed.
  - `tilesInfo.json`: Describe tiles (texture, transformation, properties such as speed, passable, etc.)
- Game data are all in json format
- No custom editor for json data (yet).

# EntityManager
-

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
