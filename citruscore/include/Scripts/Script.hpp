#pragma once

namespace Citrus {
	//Base class representing a game script
	class Script {
	public:
		//Runs every tick
		virtual void OnTick() {}

		//Runs whenever the script is activated
		virtual void OnAwake() {}

		//Get and set the active state
		const void SetActive(bool val) {
			bool prevActive = active;
			active = val;
			if(active && !prevActive) OnAwake();
		}
		const bool IsActive() { return active; }
		
		const bool operator==(Script rhs) {
            return (this == &rhs);
        }
	private:
		//Should this script run?
		//Private because we need to run the OnAwake method when this script is activated
		bool active;
	};
}