#include <util/DatabaseHandler.h>

DatabaseHandler::DatabaseHandler(int ses_id, const char* dir)
{
	directory = dir;
	session_id = ses_id;

	int exit = 0;

	exit = sqlite3_open(directory, &DB);

	sqlite3_close(DB);
}

DatabaseHandler::~DatabaseHandler()
{
	delete DB;
}

int DatabaseHandler::CreateTable(std::string sql_string)
{
	try
	{
		int exit = 0;
		exit = sqlite3_open(directory, &DB);

		char* msg_error;
		exit = sqlite3_exec(DB, sql_string.c_str(), NULL, 0, &msg_error);

		if (exit != SQLITE_OK)
		{
			k2d::KUSI_DEBUG("Error creating table at %s\n", directory);
			sqlite3_free(msg_error);
		}
		else
		{
			k2d::KUSI_DEBUG("Table created succesfully at %s\n", directory);
		}
		sqlite3_close(DB);
	}
	catch (const std::exception& e)
	{
		k2d::KUSI_DEBUG(e.what());
	}

	return 0;
}

int DatabaseHandler::InsertData(std::string sql_data)
{
	char* msg_error;

	int exit = sqlite3_open(directory, &DB);

	exit = sqlite3_exec(DB, sql_data.c_str(), NULL, 0, &msg_error);
	if (exit != SQLITE_OK)
	{
		k2d::KUSI_DEBUG("Error inserting data at %s\n Data: %s\n", directory, sql_data);
		sqlite3_free(msg_error);
	}
	else
	{
		k2d::KUSI_DEBUG("Data insert successful at %s\n", directory);
	}

	sqlite3_close(DB);

	return 0;
}


int DatabaseHandler::InsertMatchData(int match_id, int winner_id, int turns_played, std::string turn_history_string, std::string initial_board_state_str)
{
	std::string sql;

	sql = "INSERT INTO MATCHES_" + std::to_string(session_id) + "(MATCHID, WINNERID, NUM_TURNS, TURNS, INITIALBOARD) VALUES(" + std::to_string(match_id)
		+ ", " + std::to_string(winner_id) + ", " + std::to_string(turns_played) + ",'" + turn_history_string + "'," + "'" + initial_board_state_str + "'); ";

	std::cout << sql << "\n";

	InsertData(sql);

	return 0;
}

int DatabaseHandler::CreateMatchesTable()
{
	std::string sql = "CREATE TABLE IF NOT EXISTS MATCHES_" + std::to_string(session_id) + "("
		"MATCHID INTEGER PRIMARY KEY, "
		"WINNERID	INT NOT NULL, "
		"NUM_TURNS	INT NOT NULL, "
		"TURNS		TEXT, "
		"INITIALBOARD TEXT);";

	CreateTable(sql);
	return 0;
}


void DatabaseHandler::SetDirectory(const char* dir)
{
	directory = dir;
}

std::string DatabaseHandler::itoc(int value)
{
	return ("'" + std::to_string(value) + "'");
}
