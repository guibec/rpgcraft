
#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"

#include "x-thread.h"
#include "x-chrono.h"
#include "x-pad.h"
#include "x-host-ifc.h"

DECLARE_MODULE_NAME("kpad");

#include "x-MemCopy.inl"

struct AxisMapPair {
	VirtKeyBindingPair	neg;
	VirtKeyBindingPair	pos;
};

struct AxisTickPair {
	HostClockTick		neg;
	HostClockTick		pos;
};

struct AxisPressPair {
	bool			neg;
	bool			pos;
};

struct KPad_AxisEventInfo
{
	AxisTickPair	LStick_X;
	AxisTickPair	LStick_Y;

	AxisTickPair	RStick_X;
	AxisTickPair	RStick_Y;

	HostClockTick	L2;		
	HostClockTick	R2;		
};

struct KPad_AxisPressState
{
	AxisPressPair	LStick_X;		
	AxisPressPair	LStick_Y;		

	AxisPressPair	RStick_X;		
	AxisPressPair	RStick_Y;		

	bool			L2;				
	bool			R2;				
};

struct KPad_AxisMapping
{
	AxisMapPair			LStick_X;
	AxisMapPair			LStick_Y;

	AxisMapPair			RStick_X;
	AxisMapPair			RStick_Y;

	VirtKey_t			L2;
	VirtKey_t			R2;
};

static const VirtKeyBindingPair s_kdb_to_pad_btn_Default[NUM_PAD_INPUT_BUTTONS] =
{
	{ 'W',					VirtKey::Unmapped	},	// PadBtn_DPad_Up
	{ 'D',					VirtKey::Unmapped	},	// PadBtn_DPad_Right
	{ 'S',					VirtKey::Unmapped	},	// PadBtn_DPad_Down
	{ 'A',					VirtKey::Unmapped	},	// PadBtn_DPad_Left
	{ VirtKey::Escape,		VirtKey::Unmapped	},	// PadBtn_Options
	{ VirtKey::Enter,		VirtKey::Unmapped	},	// PadBtn_ViewMap

	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_TriangleY
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_CircleB	
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_CrossA		
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_SquareX
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_L1
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_R1
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_LStick
	{ VirtKey::Unmapped,	VirtKey::Unmapped	},	// PadBtn_RStick		
	
};


static const KPad_AxisMapping s_kdb_to_pad_axis_Default = []() {
	KPad_AxisMapping init;
	init.LStick_X.neg	= { VirtKey::ArrowLeft,		VirtKey::Unmapped	};
	init.LStick_X.pos	= { VirtKey::ArrowRight,	VirtKey::Unmapped	};
	init.LStick_Y.neg	= { VirtKey::ArrowUp,   	VirtKey::Unmapped	};
	init.LStick_Y.pos	= { VirtKey::ArrowDown,		VirtKey::Unmapped	};

	init.RStick_X.neg	= { 'G',					VirtKey::Unmapped	};
	init.RStick_X.pos	= { 'J',					VirtKey::Unmapped	};
	init.RStick_Y.neg	= { 'Y',					VirtKey::Unmapped	};
	init.RStick_Y.pos	= { 'H',					VirtKey::Unmapped	};

	init.L2				= { VirtKey::LCtrl		};
	init.R2				= { VirtKey::RCtrl		};
	return init;
}();

static VirtKeyBindingPair		s_kdb_to_pad_btn_map [NUM_PAD_INPUT_BUTTONS];
static KPad_AxisMapping			s_kdb_to_pad_axs_map;

#include "x-thread.h"

static PadState		s_async_pad_state;
static xMutex		s_mtx_padstate;
static thread_t		s_thr_gamepad_input;


struct AxisPressStatePair {
	bool			neg;
	bool			pos;
};

static float update_stick_axis(const HostClockTick& cur_tick, const AxisMapPair& axis_map, AxisTickPair& press_tick, AxisPressPair& prev_state)
{
	bool press_neg = Host_IsKeyPressedGlobally(axis_map.neg.primary);
	bool press_pos = Host_IsKeyPressedGlobally(axis_map.pos.primary);

	// update last_press info first to ensure subsequent delta-time checks provide expected result.

	if (press_neg) {
		if (!prev_state.neg) {	// new press event
			press_tick.neg = cur_tick;
		}
	}
	else {
		if (prev_state.neg) {	// new release event
		}
	}				

	if (press_pos) {
		if (!prev_state.pos) {	// new press event
			press_tick.pos = cur_tick;
		}
	}
	else {
		if (prev_state.pos) {	// new release event
		}
	}

	prev_state.neg = press_neg;
	prev_state.pos = press_pos;

	float newval = 0.0f;

	if (press_neg || press_pos) {
		if (press_neg && press_pos) {
			if (press_tick.neg < press_tick.pos) {
				press_pos = 0;
			}
			else {
				press_neg = 0;
			}
		}

		newval = press_neg ? -1.0f : 1.0f;
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

	auto cur_tick = Host_GetProcessTicks();

	while(1) {
		int btn_idx = -1;
		for(int btn_idx=0; btn_idx<NUM_PAD_INPUT_BUTTONS; ++btn_idx) {
			const auto& btnpair	= s_kdb_to_pad_btn_map	[btn_idx];
			auto& last_press	= s_last_press_tick_btn	[btn_idx];

			bool isPressed		= Host_IsKeyPressedGlobally(btnpair);
			bool prevPressState	= local_state.buttons.w & (1ULL << btn_idx);

			if (isPressed) {
				if (!prevPressState) {
					// new press event, record the timestamp
					last_press = cur_tick;
				}
			}
			else {
				if (prevPressState) {
					// new release event.  do something here, maybe?
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
			auto&		axis_value		= local_state.axis		.LStick;
			const auto& axis_map		= s_kdb_to_pad_axs_map	.LStick_X;
			auto&		press_time		= s_last_press_tick_axs	.LStick_X;
			auto&		prevPressState	= axis_press_state		.LStick_X;

			axis_value = update_stick_axis(cur_tick, axis_map, press_time, prevPressState);
		}

		if (1) {
			auto&		axis_value		= local_state.axis		.RStick;
			const auto& axis_map		= s_kdb_to_pad_axs_map	.RStick_X;
			auto&		press_time		= s_last_press_tick_axs	.RStick_X;
			auto&		prevPressState	= axis_press_state		.RStick_X;

			axis_value = update_stick_axis(cur_tick, axis_map, press_time, prevPressState);
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

void KPad_GetState(PadState& dest)
{
	//xScopedMutex lock(s_mtx_padstate);
	xObjCopy(dest, s_async_pad_state);
}

void KPad_CreateThread()
{
	s_mtx_padstate.Create("PadInputThread");
	thread_create(s_thr_gamepad_input, PadInputThreadProc, "PadInputPoll");
}
