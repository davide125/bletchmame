/***************************************************************************

    dlgswitches.h

    Configuration & DIP Switches customization dialog

***************************************************************************/

#pragma once

#ifndef DLGSWITCHES_H
#define DLGSWITCHES_H

#include <vector>

#include "runmachinetask.h"
#include "info.h"

class wxWindow;
struct Input;

class ISwitchesHost
{
public:
	virtual const std::vector<Input> &GetInputs() = 0;
	virtual void SetInputValue(const wxString &port_tag, ioport_value mask, ioport_value value) = 0;
};


bool show_switches_dialog(wxWindow &parent, const wxString &title, ISwitchesHost &host, Input::input_class input_class, info::machine machine);

#endif // DLGSWITCHES_H
