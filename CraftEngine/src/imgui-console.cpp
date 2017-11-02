
#include "PCH-rpgcraft.h"
#include "imgui-console.h"

ImGuiConsole	g_console;

int ImGuiConsole::TextEditCallback(ImGuiTextEditCallbackData* data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			#if 0
			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < Commands.Size; i++)
				if (Strnicmp(Commands[i], word_start, (int)(word_end-word_start)) == 0)
					candidates.push_back(Commands[i]);

			if (candidates.Size == 0)
			{
				// No match
				AddLog("No match for \"%.*s\"!\n", (int)(word_end-word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}
			#endif
		} break;

		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
				data->BufDirty = true;
			}
		} break;
	}
	return 0;
}

static int   Stricmp(const char* str1, const char* str2)         { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }

void ImGuiConsole::ClearLog()
{
	Buf.clear(); LineOffsets.clear();
	ScrollToBottom = true;
}

void ImGuiConsole::AddLog(const char* fmt, ...)
{
	int old_size = Buf.size();
	va_list args;
	va_start(args, fmt);
	Buf.appendv(fmt, args);
	va_end(args);
	for (int new_size = Buf.size(); old_size < new_size; old_size++)
		if (Buf[old_size] == '\n')
			LineOffsets.push_back(old_size);
	ScrollToBottom = true;
}

void ImGuiConsole::ExecCommand(const char* command_line)
{
	AddLog("# %s\n", command_line);

	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for (int i = History.Size-1; i >= 0; i--) {
		if (Stricmp(History[i], command_line) == 0)
		{
			free(History[i]);
			History.erase(History.begin() + i);
			break;
		}
	}
	History.push_back(_strdup(command_line));
}


void ImGuiConsole::AppendToCurrentFrame()
{
	// TODO: display items starting from the bottom

	if (ImGui::SmallButton("Clear")) { ClearLog(); }
	ImGui::SameLine();
	bool copy_to_clipboard = ImGui::SmallButton("Copy");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
	ImGui::SameLine();
	filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::PopStyleVar();
	ImGui::Separator();

	ImGui::BeginChild("ScrollingRegion", ImVec2(0,-ImGui::GetItemsLineHeightWithSpacing()*2), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopupContextWindow())
	{
		copy_to_clipboard |= ImGui::Selectable("Copy");
		if (ImGui::Selectable("Clear")) { ClearLog(); }
		ImGui::EndPopup();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
	if (copy_to_clipboard)
		ImGui::LogToClipboard();


// Here is sample code to use the frontend clipper to improve performance on huge consoles.
// This method is mutually exclusive to being used with on-the-fly filters -- the filter would
// need to be calculated in a prior step: determine # of elements, and build index table to all
// lines that are "filtered in".  The index table would provide fast random access.
//
//	ImGuiListClipper clipper(Items.Size);
//	while (clipper.Step()) {
//		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {

	if (filter.IsActive())
	{
		const char* buf_begin = Buf.begin();
		const char* line = buf_begin;
		for (int line_no = 0; line != NULL; line_no++)
		{
			const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
			if (filter.PassFilter(line, line_end)) {
				ImVec4 col = ImVec4(1.0f,1.0f,1.0f,1.0f); // A better implementation may store a type per-item. For the sample let's just parse the text.
				if (strstr(line, "[error]")) {
					col = ImColor(1.0f,0.4f,0.4f,1.0f);
				}
				elif (strncmp(line, "# ", 2) == 0) {
					col = ImColor(1.0f,0.78f,0.58f,1.0f);
				}
				ImGui::PushStyleColor(ImGuiCol_Text, col);
				ImGui::TextUnformatted(line, line_end);
				ImGui::PopStyleColor();
			}
			line = line_end && line_end[1] ? line_end + 1 : NULL;
		}
	}
	else
	{
		ImGui::TextUnformatted(Buf.begin());
	}

	if (copy_to_clipboard)
		ImGui::LogFinish();

	if (ScrollToBottom)
		ImGui::SetScrollHere();

	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	// console command line
	if (ImGui::InputText("Input", InputBuf, bulkof(InputBuf),
		ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory,
		[](ImGuiTextEditCallbackData* data) { return ((ImGuiConsole*)data->UserData)->TextEditCallback(data); }, this)
	) {
		char* input_end = InputBuf+strlen(InputBuf);
		while (input_end > InputBuf && input_end[-1] == ' ') { input_end--; } *input_end = 0;
		if (InputBuf[0])
			ExecCommand(InputBuf);
		strcpy(InputBuf, "");
	}

	ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

	// Demonstrate keeping auto focus on the input box
	if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
}

void ImGuiConsole::DrawFrame()
{
	ImGui::SetNextWindowSize(ImVec2(520,600), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Console"))
	{
		AppendToCurrentFrame();
	}
	ImGui::End();
}
