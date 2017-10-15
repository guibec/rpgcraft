
#include "PCH-framework.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"
#include "x-thread.h"
#include "x-chrono.h"
#include "x-pad.h"
#include "x-host-ifc.h"

DECLARE_MODULE_NAME("kpad");

#if 0
#	define trace_kpad(...)		log_host( __VA_ARGS__ )
#else
#	define trace_kpad(...)
#endif

#include "x-MemCopy.inl"

// ------------------------------------------------------------------------------------------------
// Note: this mapping is useful for a console-centric keyboard layout, but probably not great
// for PC-centric keyboard.  A better system for native PC keyboard is to define a set of game
// actions and then have both keyboard and pad map to that.
//
// We'll need a little bit more open-ended architecture for that:
//   * remove fixed-sized arrays that currently depend on NUM_PAD_INPUT_BUTTONS
//   * replace with open-ended array of length specified by game
//   * remove any explicit reference to PadBtn enumeration -- game will have its own enum separately.
//
// These changes are for button mapping only.  There shouldn't be any changes required to axis
// mapping except that, possibly, some of the axes won't even be bound to the keyboard in
// practice -- mouse or touch replacing certain behaviors that would be bound to r-stick on
// console gamepad, for example.
// ------------------------------------------------------------------------------------------------

const VirtKeyBindingPair g_kpad_btn_map_default[NUM_PAD_INPUT_BUTTONS] =
{
	{ VirtKey::ArrowLeft,	VirtKey::Unmapped	},	// PadBtn_DPad_Up
	{ VirtKey::ArrowRight,	VirtKey::Unmapped	},	// PadBtn_DPad_Right
	{ VirtKey::ArrowUp,		VirtKey::Unmapped	},	// PadBtn_DPad_Down
	{ VirtKey::ArrowDown,	VirtKey::Unmapped	},	// PadBtn_DPad_Left
	{ VirtKey::BackQuote,	VirtKey::Unmapped	},	// PadBtn_Options
	{ VirtKey::Tab,			VirtKey::Unmapped	},	// PadBtn_ViewMap

	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_TriangleY
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_CircleB
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_CrossA
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_SquareX
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_L1
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_R1
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_LStick
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_RStick

};


const KPad_AxisMapping g_kpad_axs_map_default = []() {
	KPad_AxisMapping init;
	init.LStick_X.neg	= { 'A',					VirtKey::Unmapped	};
	init.LStick_X.pos	= { 'D',					VirtKey::Unmapped	};
	init.LStick_Y.neg	= { 'W',					VirtKey::Unmapped	};
	init.LStick_Y.pos	= { 'S',					VirtKey::Unmapped	};

	init.RStick_X.neg	= { 'G',					VirtKey::Unmapped	};
	init.RStick_X.pos	= { 'J',					VirtKey::Unmapped	};
	init.RStick_Y.neg	= { 'Y',					VirtKey::Unmapped	};
	init.RStick_Y.pos	= { 'H',					VirtKey::Unmapped	};

	init.L2				= { VirtKey::LCtrl		};
	init.R2				= { VirtKey::RCtrl		};
	return init;
}();

static VirtKeyBindingPair	s_kpad_btn_map [NUM_PAD_INPUT_BUTTONS];
static KPad_AxisMapping		s_kpad_axs_map;

static PadState				s_async_pad_state;
static xMutex				s_mtx_padstate;
static thread_t				s_thr_gamepad_input;


struct AxisPressStatePair {
	bool			neg;
	bool			pos;
};

static float update_stick_axis(const HostClockTick& cur_tick, const KPadAxisMapPair& axis_map, KPadAxisTickPair& press_tick, KPadAxisPressPair& prev_state, const char* diagName)
{
	bool press_neg = Host_IsKeyPressedGlobally(axis_map.neg.primary);
	bool press_pos = Host_IsKeyPressedGlobally(axis_map.pos.primary);

	// update last_press info first to ensure subsequent delta-time checks provide expected result.

	if (press_neg) {
		if (!prev_state.neg) {	// new press event
			press_tick.neg = cur_tick;
			trace_kpad("Axis Event    %-10s = PRESSED", diagName);
		}
	}
	else {
		if (prev_state.neg) {	// new release event
			trace_kpad("Axis Event    %-10s = RELEASED", diagName);
		}
	}

	if (press_pos) {
		if (!prev_state.pos) {	// new press event
			press_tick.pos = cur_tick;
			trace_kpad("Axis Event    %-10s = PRESSED", diagName);
		}
	}
	else {
		if (prev_state.pos) {	// new release event
			trace_kpad("Axis Event    %-10s = RELEASED", diagName);
		}
	}

	prev_state.neg = press_neg;
	prev_state.pos = press_pos;

	float newval = 0.0f;

	if (press_neg || press_pos) {
		if (press_neg && press_pos) {
			if (press_tick.neg > press_tick.pos) {
				press_pos = 0;
			}
			else {
				press_neg = 0;
			}
		}

		newval = press_neg ? -1.0f : 1.0f;
	}

	if (newval) {
		trace_kpad("Axis Value    %-10s = %4.1f", diagName, newval);
	}
	return newval;
}

// Polls input for the game state.
static void* PadInputThreadProc(void*)
{
	PadState				local_state;
	HostClockTick			s_last_press_tick_btn	[NUM_PAD_INPUT_BUTTONS];
	KPad_AxisEventInfo		s_last_press_tick_axs;
	KPad_AxisPressState		axis_press_state;

	xMemZero(s_last_press_tick_btn);
	xMemZero(s_last_press_tick_axs);
	xMemZero(local_state);
	xMemZero(axis_press_state);


	while(1) {
		auto cur_tick = Host_GetProcessTicks();

		int btn_idx = -1;
		for(int btn_idx=0; btn_idx<NUM_PAD_INPUT_BUTTONS; ++btn_idx) {
			const auto& btnpair	= s_kpad_btn_map	[btn_idx];
			auto& last_press	= s_last_press_tick_btn	[btn_idx];

			bool isPressed		= Host_IsKeyPressedGlobally(btnpair);
			bool prevPressState	= local_state.buttons.w & (1ULL << btn_idx);

			if (isPressed) {
				if (!prevPressState) {
					// new press event, record the timestamp
					trace_kpad("Button Event  %-10s = PRESSED", enumToString((PadButtonId)btn_idx));
					last_press = cur_tick;
				}
			}
			else {
				if (prevPressState) {
					// new release event.  do something here, maybe?
					trace_kpad("Button Event  %-10s = RELEASED", enumToString((PadButtonId)btn_idx));
				}
			}

			// unconditionally update local button state
			local_state.buttons.w &= ~(1ULL				<< btn_idx);
			local_state.buttons.w |=  (uint(isPressed)	<< btn_idx);
			local_state.buttonTimestamp[btn_idx] = last_press.asSeconds();
		}

		// Translate keyboard into axis values.
		// Eaiser done using specific code implementation for each axis, rather than
		// trying to implement some loop.
		//
		// For Pos/Neg axis, favor whichever key user has pressed most recently.
		// Such that if user presses Left, then presses Right without releasing Left, favor Right.

		if (1) {
			const auto& axis_map		= s_kpad_axs_map		.LStick_X;
			auto&		press_time		= s_last_press_tick_axs	.LStick_X;
			auto&		axis_value		= local_state.axis		.LStick_X;
			auto&		prevPressState	= axis_press_state		.LStick_X;

			axis_value = update_stick_axis(cur_tick, axis_map, press_time, prevPressState, "LStick_X");
		}

		if (1) {
			const auto& axis_map		= s_kpad_axs_map		.LStick_Y;
			auto&		press_time		= s_last_press_tick_axs	.LStick_Y;
			auto&		axis_value		= local_state.axis		.LStick_Y;
			auto&		prevPressState	= axis_press_state		.LStick_Y;

			axis_value = update_stick_axis(cur_tick, axis_map, press_time, prevPressState, "LStick_Y");
		}

		if (1) {
			const auto& axis_map		= s_kpad_axs_map		.RStick_X;
			auto&		press_time		= s_last_press_tick_axs	.RStick_X;
			auto&		axis_value		= local_state.axis		.RStick_X;
			auto&		prevPressState	= axis_press_state		.RStick_X;

			axis_value = update_stick_axis(cur_tick, axis_map, press_time, prevPressState, "RStick_X");
		}

		if (1) {
			const auto& axis_map		= s_kpad_axs_map		.RStick_Y;
			auto&		press_time		= s_last_press_tick_axs	.RStick_Y;
			auto&		axis_value		= local_state.axis		.RStick_Y;
			auto&		prevPressState	= axis_press_state		.RStick_Y;

			axis_value = update_stick_axis(cur_tick, axis_map, press_time, prevPressState, "RStick_Y");
		}

		pragma_todo("Implement keyboard bindings for L2/R2 analog paddle axis");

		if (1) {
			// copy local instance pad data into the shared async buffer.
			// This is done independently of pad polling in order to minimize the amount of time
			// spent in the mutex lock.  Calls to Host_IsKeyPressedGlobally() could be time-
			// consuming depending on underlying architecture.

			// Lockless Keyboard Theory:
			//    * All members of PadState are atomics types.
			//    * Undesirable behavior could occur if the async state of the Button Press boolean
			//      doesn't match the async state of the ButtonPressTick value -- eg, the Press boolean
			//      would be read as '1' but the Tick would be stale data making it seem like it had been
			//      pressed a really-long time.
			//    * Solution: Copy tick data first, mfence(), and then copy the rest, mfence() again.
			//         First mfence ensures CPU won't serialize the button state ahead of the tick array.
			//

			//xScopedMutex lock(s_mtx_padstate);
			xObjCopy(s_async_pad_state.buttonTimestamp, local_state.buttonTimestamp);
			i_mfence();
			s_async_pad_state.buttons	= local_state.buttons;
			s_async_pad_state.axis		= local_state.axis;
			i_mfence();
		}

		xThreadSleep(8);
	}
}


//void KPad_SetButtonEventCount(int count) {
// TODO: Create this, to give game open-ended control over supported keyboard command events.
//       (see comment at top of gile for details)
//}

void KPad_GetState(PadState& dest)
{
	// See notes in thread keyboard polling thread for why mutex lock has been removed.
	//xScopedMutex lock(s_mtx_padstate);
	xObjCopy(dest, s_async_pad_state);
}

void KPad_SetMapping(const VirtKeyBindingPair (&newmap)[NUM_PAD_INPUT_BUTTONS])
{
	xObjCopy(s_kpad_btn_map, newmap);
}

void KPad_SetMapping(const KPad_AxisMapping& newmap)
{
	xObjCopy(s_kpad_axs_map, newmap);
}

void KPad_GetMapping(VirtKeyBindingPair (&dest)[NUM_PAD_INPUT_BUTTONS])
{
	xObjCopy(dest, s_kpad_btn_map);
}

void KPad_GetMapping(KPad_AxisMapping& dest)
{
	xObjCopy(dest, s_kpad_axs_map);
}


void KPad_CreateThread()
{
	s_mtx_padstate.Create("PadInputThread");
	thread_create(s_thr_gamepad_input, PadInputThreadProc, "PadInputPoll");
}
