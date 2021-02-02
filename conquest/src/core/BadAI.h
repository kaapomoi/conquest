#pragma once
#include <core/Engine.h>
#include <core/AI.h>

class BadAI : public AI
{
public:
	BadAI(int id, ServerSim* server_sim);
	virtual ~BadAI();

	void Update() override;

	void SendInputToServer(int input_num) override;

	void GetTakenColorsFromServer() override;

	Event GetNextEventFromServer() override;

protected:

	// Random engine
	std::mt19937 rand_engine;
};