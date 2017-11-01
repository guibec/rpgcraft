#pragma once

#include "x-types.h"
#include "imgui.h"

struct ImGuiConsole
{
	ImGuiTextBuffer			Buf;
	ImVector<int>			LineOffsets;        // Index to lines offset
	ImVector<const char*>	Commands;
	ImVector<char*>			History;
	int						HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	char					InputBuf[256] = {0};
	bool					ScrollToBottom;
	ImGuiTextFilter			filter;

	void	AddLog					(const char* fmt, ...) IM_FMTARGS(2);
	void	ClearLog				();
	int		TextEditCallback		(ImGuiTextEditCallbackData* data);
	void	ExecCommand				(const char* command_line);
	void	AppendToCurrentFrame	();
	void	DrawFrame				();
};

extern ImGuiConsole	g_console;
