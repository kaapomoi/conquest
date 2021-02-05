#include <core/EventQueue.h>

EventQueue::EventQueue()
{

}

EventQueue::~EventQueue()
{

}

void EventQueue::AddClientsToEvent(Event e, int client_id)
{
	for (int i = 0; i < events.size(); ++i)
	{
		// If we found the event, add the client id to it
		if (e.GetId() == events.at(i).GetId())
		{
			events.at(i).SubscribeAClientId(client_id);
		}
	}
}

void EventQueue::AddItem(Event e)
{
	events.push_back(e);
}

Event EventQueue::GetNextEvent(int client_id)
{
	if (events.size() > 0)
	{

		// Get the next event 
		Event& e = events.front();

		//// Returns the first event whose client ids contain client_id
		//auto it = std::find_if(events.begin(), events.end(), [client_id](Event& ev) { 
		//	std::vector<int> v = ev.GetClientIds();
		//	if (v.empty())
		//	{
		//		return false;
		//	}
		//	// If the client ids are not empty, try to find the client_id
		//	if (std::find(v.begin(), v.end(), client_id) != v.end()) {
		//		return true;
		//	}
		//	else {
		//		return false;
		//	}
		//});
		//
		//// Remove the subscription from the event
		
		e.RemoveClientID(client_id);
	
		// Return the event
		return e;
	}
	return Event(EventType::NULLEVENT, -1, std::vector<int> {});
}

Event EventQueue::PopFirst()
{
	Event e = events.front();
	events.erase(events.begin());

	return e;
}

void EventQueue::Update()
{
	if (events.size() > 0)
	{
		// If everyone that is suppposed to read the message have read the message, remove it from the queue
		auto it = std::find_if(events.begin(), events.end(), [](Event& ev) {
			std::vector<int> v = ev.GetClientIds();
			if (v.empty())
			{
				return true;
			}
		});
		if (it != events.end())
		{
			events.erase(it);
		}
	}
}
