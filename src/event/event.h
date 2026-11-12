#pragma once
#include <aqua/event.h>
#include <SDL3/SDL.h>

// Native events stay internal and are delivered to backend tooling before translation.
bool aqua_event_poll(AquaEvent* event, AquaRenderer* renderer, bool process_imgui_input);
