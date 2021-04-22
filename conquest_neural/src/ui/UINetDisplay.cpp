#include <ui/UINetDisplay.h>

UINetDisplay::UINetDisplay(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::GLTexture texture, k2d::SpriteBatch* sb, k2d::Application* app):
	UIBase(name, position, size, depth)
{
	this->node_size = 0;
	this->weight_size = 0;
	this->texture = texture;
	this->sb = sb;
	this->app = app;
	this->net = nullptr;
}

UINetDisplay::~UINetDisplay()
{
	for (k2d::Sprite* s : node_sprites)
	{
		delete s;
	}
	for (k2d::Sprite* s : weight_sprites)
	{
		delete s;
	}
}

void UINetDisplay::Update(double dt)
{
	if (net)
	{
		UpdateAlphas();
		for (k2d::Sprite* s : node_sprites)
		{
			s->Tick();
		}
		for (k2d::Sprite* s : weight_sprites)
		{
			s->Tick();
		}
	}
	if (background_sprite)
	{
		background_sprite->Tick();
	}
}

void UINetDisplay::SetNeuralNetPtr(NeuralNet* nn)
{
	for (k2d::Sprite* s : node_sprites)
	{
		delete s;
	}
	for (k2d::Sprite* s : weight_sprites)
	{
		delete s;
	}
	node_sprites.clear();
	weight_sprites.clear();

	this->net = nn;
	std::vector<std::vector<Neuron>>& layers = net->GetLayers();

	float start_y = position.y - size.y * 0.5f;
	float start_x = position.x - size.x * 0.5f;

	float node_height = (size.y) * 0.75f / layers[0].size();
	k2d::clamp(node_height, 1.0f, 100.0f);
	float node_width = node_height;
	
	float gap_width = size.x / (layers.size()-1);

	// Calculate positions for the nodes
	for (size_t i = 0; i < layers.size(); i++)
	{
		float gap_height = size.y / layers[i].size();

		start_y = position.y - size.y * 0.5f + gap_height * 0.5f;//(layers[0].size() - layers[i].size()) * (node_height + gap_height) / 4.0f;
		node_positions.push_back(std::vector<k2d::vf2d>());
		for (size_t j = 0; j < layers[i].size(); j++)
		{
			node_positions.back().push_back(k2d::vf2d(start_x, start_y));
			start_y += gap_height;
		}
		start_x += gap_width;
	}

	for (size_t i = 0; i < layers.size(); i++)
	{
		topology.push_back(layers[i].size());
		for (size_t j = 0; j < layers[i].size(); j++)
		{
			// Push back node sprite 
			node_sprites.push_back(new k2d::Sprite(node_positions[i][j], k2d::vf2d(node_width, node_height), depth, k2d::Color(255), texture, sb));

			// Dont make weight sprites for the last layer (output layer)
			if (i < layers.size()- 1)
			{
				// Weight sprites 
				for (size_t k = 0; k < layers[i+1].size(); k++)
				{
					weight_sprites.push_back(app->FromToLineSprite(node_positions[i][j] + k2d::vf2d(2.0f, 0.0f), node_positions[i+1][k] - k2d::vf2d(2.0f, 0.0f), depth, "full_i"));
				}
			}
		}
	}
}

void UINetDisplay::UpdateAlphas()
{
	static const int layer_alphas[] = {30, 50, 128};
	if (net)
	{
		std::vector<std::vector<Neuron>>& layers = net->GetLayers();
		int node_index = 0;
		int weight_index = 0;
		for (size_t i = 0; i < layers.size(); i++)
		{
			for (size_t j = 0; j < layers[i].size(); j++)
			{
				int alpha_node = 0;
				if (weights_only_mode)
				{
					alpha_node = k2d::map_to_range(layers[i].at(j).bias_weight, -1, 1, 0, 255);
				}
				else
				{
					alpha_node = k2d::map_to_range(layers[i].at(j).output_value, -1, 1, 0, 255);
				}
				node_sprites.at(node_index)->SetColor(k2d::Color(255, 0, 255, alpha_node));
				node_index++;

				// Dont make weight sprites for the last layer (output layer)
				if (i < layers.size() - 1)
				{
					// Weight sprites 
					for (size_t k = 0; k < layers[i + 1].size(); k++)
					{
						int alpha_weight = 0;
						if (weights_only_mode)
						{
							alpha_weight = k2d::map_to_range(layers[i].at(j).output_weights.at(k), -1, 1, 0, layer_alphas[i]);
						}
						else
						{
							alpha_weight = k2d::map_to_range(layers[i].at(j).output_weights.at(k) * layers[i].at(j).output_value, -1, 1, 0, layer_alphas[i]);

						}
						weight_sprites.at(weight_index)->SetColor(k2d::Color(255, 0, 255, alpha_weight));
						weight_index++;
					}
				}
			}
		}
	}
}

void UINetDisplay::AddBackground(k2d::Color color)
{
	if (!background_sprite)
	{
		background_sprite = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, size.y, depth - 1.0f, glm::vec4(0, 0, 1, 1), color, texture, sb);
	}
	else
	{
		background_sprite->SetColor(color);
	}
}

void UINetDisplay::OnClick()
{
	UIClickable::OnClick();
}

void UINetDisplay::ToggleWeightsOnlyMode()
{
	weights_only_mode = !weights_only_mode;
}
