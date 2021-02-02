#pragma once
#include <core/Engine.h>
#include <core/Event.h>

/// <summary>
/// First in first out structure for Events
/// </summary>
class EventQueue
{
public:
	EventQueue();
	~EventQueue();

	// Adds a client to the event
	void AddClientsToEvent(Event e, int id);

	// Adds an item to the queue
	void AddItem(Event e);

	// Gets the next event and unsubscribes the client from the event
	Event GetNextEvent(int client_id);

	void Update();

private:
	Event PopFirst();
	
	std::vector<Event> events;
};