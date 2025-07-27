#pragma once
#include "Share/Rosemary.h"

namespace AntiCheat{
	void RemoveSecurityChecks(Rosemary& r);
	bool RemoveDisconnectException(Rosemary& r);
	bool RemoveNewPatcherCheck(Rosemary& r);
	bool RemoveSecurityClient(Rosemary& r);
	bool RemoveSecurityClientInitFailedException(Rosemary& r);
}