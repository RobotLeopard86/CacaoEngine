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
			bool prevActive = active;
			active = val;
			if(active && !prevActive) {
				OnActivate();
				return;
			}
			if(!active && prevActive) {
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
	};
}