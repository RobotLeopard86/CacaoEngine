#pragma once

#define CACAOST_GET(c)      \
	c& c::Get() {           \
		static c _instance; \
		return _instance;   \
	}