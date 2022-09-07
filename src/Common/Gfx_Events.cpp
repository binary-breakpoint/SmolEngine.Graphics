#include "Gfx_Precompiled.h"
#include "Common/Gfx_Events.h"

namespace SmolEngine
{
	void Gfx_EventSender::SendEvent(Gfx_Event & event, Gfx_Event::Type eventType, Gfx_Event::Category eventCategory, int action, int key)
	{
		event.m_EventType = eventType; event.m_EventCategory = eventCategory; event.m_Key = key; event.m_Action = action; event.m_Handled = false;
		OnEventFn(event);
	}

	Gfx_Event::Gfx_Event()
		:
		m_Handled{false},
		m_Key{0},
		m_Action{0}
	{

	}

	bool Gfx_Event::IsType(Gfx_Event::Type type)
	{
		return m_EventType == type;
	}

	bool Gfx_Event::IsCategory(Gfx_Event::Category category)
	{
		return m_EventCategory == category;
	}

	bool Gfx_Event::IsEventReceived(Gfx_Event::Type type, Gfx_Event& event)
	{
		return event.m_EventType == type;
	}
}
