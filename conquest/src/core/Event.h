#pragma once
#include <core/Engine.h>

enum class EventType
{
	TURN_CHANGE = 0,
	GAME_OVER,
	INVALID_COLOR,
	NULLEVENT
};

class Event
{
public:
	Event(EventType et, int id, std::vector<int> init_client_ids);
	~Event();

	// Subscribe a client to this event
	void SubscribeAClientId(int client_id);

	// Unsubscribe from the event
	void RemoveClientID(int client_id);

	// Adds the separator ":" + the data string to the data string 
	void AppendToData(std::string appendable);

	// Sets the Data string
	void SetData(std::string data);

	// Gets the id
	int GetId();

	// Is ready to remove when all clients have been unsubscribed from the event
	bool ReadyToRemove();

	std::string GetData();
	EventType GetType();

protected:
	// This is a running id
	int id;
	// Data string, eg. "<player_id>:<num_turns>", "12784,52" 
	std::string data;

	// Store the event type, useful for decoding the data string
	EventType event_type;

	// Subscribed client ids
	std::vector<int> client_ids;
};
