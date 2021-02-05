#pragma once
#include <core/Engine.h>
#include <util/sqlite3.h>

class DatabaseHandler
{
public:
	DatabaseHandler(int session_id, const char* dir);
	~DatabaseHandler();
	// static AddToDatabase(data)
	int InsertMatchData(int match_id, int winner_id, int turns_played, std::vector<int> turn_history);
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