#pragma once

#include "World/Component.hpp"

namespace Cacao {
	//Base class representing a game script
	class Script : public Component {
	  public:
		//Runs every tick
		virtual void OnTick(double timestep) {}

		//Runs every fixed tick
		virtual void OnFixedTick() {}

		//Runs whenever the script is activated
		virtual void OnActivate() {}

		//Runs whenever the script is deactivated
		virtual void OnDeactivate() {}

		//Get and set the active state
		void SetActive(bool val) override final {
			bool prevActive = this->IsActive();
			_SetActiveInternal(val);
			if(this->IsActive() && !prevActive) {
				OnActivate();
				return;
			}
			if(!this->IsActive() && prevActive) {
				OnDeactivate();
				return;
			}
		}

		std::string GetKind() override final {
			return "SCRIPT";
		}

		const bool operator==(Script rhs) {
			return (this == &rhs);
		}

		//Virtual destructor
		virtual ~Script() {}
	};
}