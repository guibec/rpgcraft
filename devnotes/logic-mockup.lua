
-- IMPORTANT: Don't really like this mockup anymore.  Too heavyweight in the Lua-side.
--   Going to run with a more CPP-driven model and patch in Lua callbacks at specific
--   points, to retrieve "script-configurable variable states" and just see how that
--   goes instead.
--
--   Keeping this here just for near-by future reference.  (Delete it if it's become 2018!!)
--
-- BEGIN DEPRECIATED SECTION ------------------------
--
-- TODO:
--   BeginScene becomes LogicScene, move to separate module scope  (own directory?)
--   Create _ENV.g_NetworkState - Automatically serialized and network-transferred by C++ layer.
--   Lua code to convert _ENV.g_NetworkState -> _ENV.g_LocalState  .. ?
--   
-- Considerations:
--   * We just want Lua for the rapid-iteration portions!  on-the-fly adjustment of things like:
--     * [logic] player response to input
--     * [logic] NPC behavior and response to player
--     * [render] environment effects (lighting, particles, etc)
--     * [render] Post-process effect parameters
--     * 

if false then

	-- Resource Loading Section -----------------------


	local tilesheet1 = Engine.LoadTexture("..\\rpg_maker_vx__modernrtp_tilea2_by_painhurt-d3f7rwg.png")

	local vs_tile			= Engine.LoadShaderVS("TileEngine.fx", "VS")

	local ps_tile_unlit		= Engine.LoadShaderPS("TileEngine.fx", "PS_unlit")
	local ps_tile_lit		= Engine.LoadShaderPS("TileEngine.fx", "PS_lit")

	local vs_world_effect	= Engine.LoadShaderVS("WorldEffect.fx", "VS")
	local ps_world_effect	= Engine.LoadShaderPS("WorldEffect.fx", "PS")

	local screenX, screenY 	= Engine.GetDisplaySizePx()
	
	-- Always render full scene into large render target, and then blut just the visible (zoomed) portion into
	-- the user's viewport.
	local rt_TileMap 		= Engine.CreateRenderTarget(screenX*1.5, screenY*1.5)


	-- END Resource Loading Section -------------------
	

	-- BeginScene and DrawScene are meant to run in parallel using automatic behind-the-scenes double-buffer.

	local BeginScene = function()
		TileEngine.SetTileMap()

		--  ??  How best to manage scene lighting ?
		TileEngine.SetAmbientLight(0.75f)
		TileEngine.AddLightSource(x,y,color,intensity,etc)
	end
	
	local DrawScene = function()
		local rt_backbuffer = Engine.GetBackbufferRenderTarget()
	
		SetInputLayout(InputLayout_SomethingHere)
		TileEngine.SetTileSheet(tilesheet1)
		TileEngine.BindShaders(vs_tile, ps_tile_unlit)
		TileEngine.RenderToTarget(renderTarget)
		
		-- Set up full-screen quad and draw tiled render target
		TileEngine.BindShaders(vs_world_effect, ps_world_effect)
		Engine.DrawQuad(rt_TileMap, srcUV, rt_backbuffer, dstXY)		-- src, src, dst, dst
		
		-- Draw UI Overlay Elements onto backbuffer.
	end
	
	
	Engine.SetCallback_DrawScene(DrawScene)
	Engine.SetCallback_BeginScene(BeginScene)
end


-- Some Remarks For later:
--    Engine.GetDisplaySizePx() -
--        - returns best-guess ideal of the user's display size.  May or may not precisely match
--          the backbuffer display area which is obtained during BeginScene or DrawScene.
--
