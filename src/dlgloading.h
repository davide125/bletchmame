/***************************************************************************

	dlgloading.h

	"loading MAME Info..." modal dialog

***************************************************************************/

#pragma once

#ifndef DLGLOADING_H
#define DLGLOADING_H

#include <wx/window.h>
#include <functional>

bool show_loading_mame_info_dialog(wxWindow &parent, const std::function<bool()> &poll_completion_check);

#endif // DLGLOADING_H