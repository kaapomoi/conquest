#pragma once
#include <core/Engine.h>
#include <util/sqlite3.h>

class DatabaseHandler
{
public:
	DatabaseHandler(int session_id, const char* dir);
	~DatabaseHandler();
	// static AddToDatabase(data)
	int InsertMatchData(int match_id, int winner_id, int turns_played, int p0_id, int p1_id, std::string turn_history_string, std::string initial_board_state_str);
	int CreateMatchesTable();

	void SetDirectory(const char* dir);
private:
	int CreateTable(std::string sql_string);
	int InsertData(std::string sql_data);

	// Trash int to 'int'
	std::string itoc(int value);

	const char* directory;

	int session_id;

	sqlite3* DB;
};