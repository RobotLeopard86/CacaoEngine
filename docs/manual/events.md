# Built-In Event List

The below are all of the built-in event types in Cacao Engine.
| Event ID | Trigger | Type |
| -------- | ------- | ---- |
| `EngineShutdown` | The engine is shutting down | `Event` |
| `WindowClose` | The window is closing | `Event` |
| `WindowResize` | The size of the window has changed | `DataEvent<glm::uvec2>` |
| `MouseMove` | The cursor has moved | `DataEvent<glm::dvec2>` |
| `MouseScroll` | The mouse wheel has been scrolled | `DataEvent<glm::dvec2>` |
| `MousePress` | A mouse button has been pressed | `DataEvent<unsigned int>` |
| `MouseRelease` | A mouse button has been released | `DataEvent<unsigned int>` |
| `KeyDown` | A key has been pressed | `DataEvent<unsigned int>` |
| `KeyUp` | A key has been released | `DataEvent<unsigned int>` |