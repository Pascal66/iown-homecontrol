﻿local var_0_0 = require("Overkiz.HomeAutomation.Shared.IoHomecontrolOvp.Parameter.Definition")
local var_0_1 = require("Overkiz.HomeAutomation.Shared.IoHomecontrolOvp.Command.commandModel")
local var_0_2 = {
	startup = true,
	info = {
		parameter = 5,
		value = {
			type = "ay",
			data = {
				2,
				166,
				3
			}
		}
	}
}
local var_0_3 = var_0_0(84059651, var_0_1.access.READ, var_0_2)

var_0_3.refresh = var_0_2

return var_0_3
