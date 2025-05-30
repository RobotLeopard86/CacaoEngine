#include "Cacao/Script.hpp"

namespace Cacao {
	void Script::OnEnableStateChange() {
		//Run the appropriate hooks
		if(IsEnabled()) {
			OnEnable();
		} else {
			OnDisable();
		}
	}
}