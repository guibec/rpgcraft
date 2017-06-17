
WorldView = {}

WorldView.getMeshSize = function(formFactor, displayResX, displayResY)

	-- DPI is not provided because it doesn't translate well across desktop/mobile/console devices anyway.
	-- (eg, DPI is kind of useless info unless you also know how far the user is from their screen)

	-- DisplayResXY, *maximum* combined non-windowed display resolution, useful for determining user display
	-- aspect ratio.
	--   Windowed mode   - all displays attached to a system are considered.
	--   Fullscreen mode - only the current display is considered.
	--
	-- Windowed resolution is not provided.  View will be scaled or cropped to fit window backbuffer
	-- depending on user setting.  A single world view size is used to simplify window size changes.

	-- The engine internally will display about 90% of available view at max-zoom-out.
	-- Ideal target is a max 2:1 zoom at 1920x1080 on a desktop


	local x,y = 24,24		-- 2048, 2048
	local aspect = displayResX / displayResY
	
	if DevForce ~= nil then
		if DevForce.FormFactor ~= nil then
			formFactor = DevForce.FormFactor
		end
	
		if DevForce.WorldView.MeshSize ~= nil then
			return DevForce.WorldView.MeshSize.x, DevForce.WorldView.MeshSize.y
		end
	end
	
	if true then
		-- for now: just use a fixed square box.
		return x,y
	end

	-- in the future: these can help optimize buffer sizes and possibly improve user experience...
	
	if formFactor == "desktop" then
		if aspect <= 0.75 then
			-- One or more displays is clearly rotated vertically
			return 1280, 1280 / aspect

		elseif aspect >= 1.25 then
			return 2048, 2048
			
		else
			-- square display or some combination of variously-oriented monitors
			return 2048, 2048
		end

	-- Phones and tablets can rotate, so just make x/y the same number
	elseif formFactor == "phone" then
		 return 1024, 1024

	elseif formFactor == "tablet" then
		return 1280, 1280
	end
end
