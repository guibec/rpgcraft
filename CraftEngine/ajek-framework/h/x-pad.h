#pragma once

#include "x-types.h"
#include "x-chrono.h"
#include "x-host-ifc.h"

// --------------------------------------------------------------------------------------
//  PadInputButtons / PadInputAxis  (enum)
// --------------------------------------------------------------------------------------
// the following pad map should be adhered to, in order to maintain future-friendliness to
// porting the title to console.

enum PadButtonId
{
	PadBtnId_DPad_Up,
	PadBtnId_DPad_Right,
	PadBtnId_DPad_Down,
	PadBtnId_DPad_Left,

	PadBtnId_TriangleY,			// Triangle (Y)
	PadBtnId_CircleB,			// Circle   (B)
	PadBtnId_CrossA,			// Cross    (A)
	PadBtnId_SquareX,			// Square   (X)

	PadBtnId_Options,			// start on PS3, Menu on XB1
	PadBtnId_ViewMap,			// PAD on PS4, select on PS3, View on XB1

	PadBtnId_L1,
	PadBtnId_R1,
	PadBtnId_LStick,			// Click the Stick! also known as L3 (PS3/PS4)
	PadBtnId_RStick,			// Click the Stick! also known as R3 (PS3/PS4)

	NUM_PAD_INPUT_BUTTONS
};

//extern const char* enum_toString(const PadButtonId& id);

static inline const char* enumToString(const PadButtonId& id)
{
	// devnote: would prefer this in a pad.cpp file, once one exists...

	static const int o = bulkof("PadBtnId");
	switch(id) {
		CaseReturnString2(o, PadBtnId_DPad_Up		);
		CaseReturnString2(o, PadBtnId_DPad_Right	);
		CaseReturnString2(o, PadBtnId_DPad_Down		);
		CaseReturnString2(o, PadBtnId_DPad_Left		);
		CaseReturnString2(o, PadBtnId_TriangleY		);
		CaseReturnString2(o, PadBtnId_CircleB		);
		CaseReturnString2(o, PadBtnId_CrossA		);
		CaseReturnString2(o, PadBtnId_SquareX		);
		CaseReturnString2(o, PadBtnId_Options		);
		CaseReturnString2(o, PadBtnId_ViewMap		);
		CaseReturnString2(o, PadBtnId_L1			);
		CaseReturnString2(o, PadBtnId_R1			);
		CaseReturnString2(o, PadBtnId_LStick		);
		CaseReturnString2(o, PadBtnId_RStick		);

		default:
			unreachable_qa("invalid pad button id");
	}
	return "unknown";
}


static const u32		PadBtnMsk_DPad_Up		= (1UL << PadBtnId_DPad_Up		);
static const u32		PadBtnMsk_DPad_Right	= (1UL << PadBtnId_DPad_Right	);
static const u32		PadBtnMsk_DPad_Down		= (1UL << PadBtnId_DPad_Down	);
static const u32		PadBtnMsk_DPad_Left		= (1UL << PadBtnId_DPad_Left	);
static const u32		PadBtnMsk_TriangleY		= (1UL << PadBtnId_TriangleY	);
static const u32		PadBtnMsk_CircleB		= (1UL << PadBtnId_CircleB		);
static const u32		PadBtnMsk_CrossA		= (1UL << PadBtnId_CrossA		);
static const u32		PadBtnMsk_SquareX		= (1UL << PadBtnId_SquareX		);
static const u32		PadBtnMsk_Options		= (1UL << PadBtnId_Options		);
static const u32		PadBtnMsk_ViewMap		= (1UL << PadBtnId_ViewMap		);
static const u32		PadBtnMsk_L1			= (1UL << PadBtnId_L1			);
static const u32		PadBtnMsk_R1			= (1UL << PadBtnId_R1			);
static const u32		PadBtnMsk_LStick		= (1UL << PadBtnId_LStick		);
static const u32		PadBtnMsk_RStick		= (1UL << PadBtnId_RStick		);


union PadButtonState
{
	u32			w;

	struct {
		u32		DPad_Up			: 1;
		u32		DPad_Right		: 1;
		u32		DPad_Down		: 1;
		u32		DPad_Left		: 1;

		u32		TriangleY		: 1;		// Triangle (Y)
		u32		CircleB			: 1;		// Circle   (B)
		u32		CrossA			: 1;		// Cross    (A)
		u32		SquareX			: 1;		// Square   (X)

		u32		Options			: 1;		// start on PS3, Menu on XB1
		u32		ViewMap			: 1;		// PAD on PS4, select on PS3, View on XB1

		u32		L1				: 1;
		u32		R1				: 1;
		u32		LStick			: 1;
		u32		RStick			: 1;
	};
};

struct PadAxisState
{
	float LStick_X;		// range -1.0f to 1.0f
	float LStick_Y;		// range -1.0f to 1.0f
	float RStick_X;		// range -1.0f to 1.0f
	float RStick_Y;		// range -1.0f to 1.0f

	float L2;			// range 0.0f to 1.0f
	float R2;			// range 0.0f to 1.0f
};

struct PadState
{
	PadAxisState		axis;
	PadButtonState		buttons;
	float				buttonTimestamp	[NUM_PAD_INPUT_BUTTONS];
};


// KPad - Keyboard-to-Pad Input Interface
// (may be moved to its own header in the future)

struct KPadAxisMapPair {
	VirtKeyBindingPair	neg;
	VirtKeyBindingPair	pos;
};

struct KPadAxisTickPair {
	HostClockTick		neg;
	HostClockTick		pos;
};

struct KPadAxisPressPair {
	bool				neg;
	bool				pos;
};

struct KPad_AxisEventInfo
{
	KPadAxisTickPair	LStick_X;
	KPadAxisTickPair	LStick_Y;

	KPadAxisTickPair	RStick_X;
	KPadAxisTickPair	RStick_Y;

	HostClockTick		L2;
	HostClockTick		R2;
};

struct KPad_AxisPressState
{
	KPadAxisPressPair	LStick_X;
	KPadAxisPressPair	LStick_Y;

	KPadAxisPressPair	RStick_X;
	KPadAxisPressPair	RStick_Y;

	bool				L2;
	bool				R2;
};

struct KPad_AxisMapping
{
	KPadAxisMapPair			LStick_X;
	KPadAxisMapPair			LStick_Y;

	KPadAxisMapPair			RStick_X;
	KPadAxisMapPair			RStick_Y;

	VirtKey_t				L2;
	VirtKey_t				R2;
};


extern void		KPad_SetMapping			(const VirtKeyBindingPair (&newmap)[NUM_PAD_INPUT_BUTTONS]);
extern void		KPad_SetMapping			(const KPad_AxisMapping& newmap);
extern void		KPad_GetMapping			(VirtKeyBindingPair (&dest)[NUM_PAD_INPUT_BUTTONS]);
extern void		KPad_GetMapping			(KPad_AxisMapping& dest);

extern void		KPad_GetState			(PadState& dest);
extern void		KPad_CreateThread		();
extern void		KPad_SetKeyboardFocus	(bool focus);



extern const VirtKeyBindingPair		g_kpad_btn_map_default[NUM_PAD_INPUT_BUTTONS];
extern const KPad_AxisMapping		g_kpad_axs_map_default;
