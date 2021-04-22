#pragma once
#include <core/Engine.h>
#include <ai/NeuralAI.h>

class NeuralSaver
{
public:
	static void SaveNeuralAiToFile(const char* file_name, NeuralAI* ai);

};