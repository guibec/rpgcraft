
DbgConsole = {}

DbgConsole.Texture      = "..\\8x8font.png"     -- unused currently (uses builtin 8x8)
DbgConsole.CharSize     = { 8, 8 }
DbgConsole.SheetSize    = { 1280, 960 } -- 1280 for 4:3 ratio console, 960 to leave some margin at the bottom.

DbgConsole.Shader       = { "DbgFont.fx", "VS", "PS" }

-- DbgConsole.Shader        = { Filename="DbgFont.fx", CallVS="VS", CallFS="FS" }
