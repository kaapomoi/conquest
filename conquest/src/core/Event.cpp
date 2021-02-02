#include <core/Event.h>

Event::Event(EventType et, int id, std::vector<int> init_client_ids)
	: event_type(et), id(id)
{
	for (size_t i = 0; i < init_client_ids.size(); i++)
	{
		client_ids.push_back(init_client_ids.at(i));
	}
}

Event::~Event()
{

}

void Event::SubscribeAClientId(int client_id)
{
	client_ids.push_back(client_id);
}

void Event::RemoveClientID(int client_id)
{
	// Save the index
	client_ids.erase(std::remove(client_ids.begin(), client_ids.end(), client_id), client_ids.end());
}

void Event::AppendToData(std::string appendable)
{
	this->data.append(":" + appendable);
}

void Event::SetData(std::string data)
{
	this->data = data;
}

int Event::GetId()
{
	return id;
}

bool Event::ReadyToRemove()
{
	// Safe to remove is size of vector is 0 or less
	return client_ids.size() <= 0;
}

std::string Event::GetData()
{
	return data;
}

EventType Event::GetType()
{
	return event_type;
}
