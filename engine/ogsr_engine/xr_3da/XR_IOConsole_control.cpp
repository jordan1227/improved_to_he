////////////////////////////////////////////////////////////////////////////
// Module : XR_IOConsole_control.cpp
// Created : 03.10.2008
// Author : Evgeniy Sokolov
// Description : Console`s control-functions class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XR_IOConsole.h"
#include "line_editor.h"


constexpr LPCSTR cmd_history_file = "console_history.txt";
constexpr u32 cmd_history_saved = 7;

void CConsole::push_cmd_history(shared_str const& str)
{
	if (str.size() == 0)
	{
		return;
	}
	auto it = std::find(m_cmd_history.begin(), m_cmd_history.end(), str);
	if (it != m_cmd_history.end())
	{
		m_cmd_history.erase(it);
	}
	m_cmd_history.push_back(str);
	if (m_cmd_history.size() > m_cmd_history_max)
	{
		m_cmd_history.erase(m_cmd_history.begin());
	}
}

void CConsole::add_cmd_history(shared_str const& str)
{
	if (str.size() == 0)
	{
		return;
	}
	push_cmd_history(str);
	save_cmd_history();
}

void CConsole::load_cmd_history()
{
	if (!FS.exist(fsgame::app_data_root, cmd_history_file))
	{
		return;
	}
	IReader* F = FS.r_open(fsgame::app_data_root, cmd_history_file);
	if (!F)
	{
		return;
	}
	while (!F->eof())
	{
		xr_string line;
		F->r_string(line);
		xr_string_utils::trim(line);
		if (!line.empty())
		{
			push_cmd_history(line.c_str());
		}
	}
	FS.r_close(F);
}

void CConsole::save_cmd_history()
{
	IWriter* F = FS.w_open(fsgame::app_data_root, cmd_history_file);
	if (!F)
	{
		return;
	}
	size_t from = m_cmd_history.size() > cmd_history_saved ? m_cmd_history.size() - cmd_history_saved : 0;
	for (size_t i = from; i < m_cmd_history.size(); ++i)
	{
		F->w_string(m_cmd_history[i].c_str());
	}
	FS.w_close(F);
}

void CConsole::next_cmd_history_idx()
{
	--m_cmd_history_idx;
	if (m_cmd_history_idx < 0)
	{
		m_cmd_history_idx = 0;
	}
}

void CConsole::prev_cmd_history_idx()
{
	++m_cmd_history_idx;
	if (m_cmd_history_idx >= (int)m_cmd_history.size())
	{
		m_cmd_history_idx = m_cmd_history.size() - 1;
	}
}

void CConsole::reset_cmd_history_idx()
{
	m_cmd_history_idx = -1;
}

bool CConsole::is_cmd_history_selected()
{
	if (m_cmd_history_idx < 0 || m_cmd_history_idx >= (int)m_cmd_history.size())
	{
		return false;
	}
	return xr_strcmp(ec().str_edit(), m_cmd_history[m_cmd_history.size() - 1 - m_cmd_history_idx].c_str()) == 0;
}

void CConsole::next_selected_tip()
{
	++m_select_tip;
	check_next_selected_tip();
}

void CConsole::check_next_selected_tip()
{
	if (m_select_tip >= (int)m_tips.size())
	{
		m_select_tip = m_tips.size() - 1;
	}

	int sel_dif = m_select_tip - VIEW_TIPS_COUNT + 1;
	if (sel_dif < 0)
	{
		sel_dif = 0;
	}

	if (sel_dif > m_start_tip)
	{
		m_start_tip = sel_dif;
	}
}

void CConsole::prev_selected_tip()
{
	--m_select_tip;
	check_prev_selected_tip();
}

void CConsole::check_prev_selected_tip()
{
	if (m_select_tip < 0)
	{
		m_select_tip = 0;
	}

	if (m_start_tip > m_select_tip)
	{
		m_start_tip = m_select_tip;
	}
}

void CConsole::reset_selected_tip()
{
	m_select_tip = -1;
	m_start_tip = 0;
	m_disable_tips = false;
}
