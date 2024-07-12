#include <iostream>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip> // Для std::put_time

#define SQL_RESULT_LEN 4000
#define SQL_RETURN_CODE_LEN 2000

class Database {
private:
    SQLHANDLE sqlConnHandle;
    SQLHANDLE sqlStmtHandle;
    SQLHANDLE sqlEnvHandle;

public:
    Database() : sqlConnHandle(NULL), sqlStmtHandle(NULL), sqlEnvHandle(NULL) {}

    ~Database() {
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
        SQLDisconnect(sqlConnHandle);
        SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
        SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
    }

    bool connect(const std::string& connectionStringFile) {
        SQLWCHAR retconstring[SQL_RETURN_CODE_LEN];

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle))
            return false;

        if (SQL_SUCCESS != SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle))
            return false;

        std::ifstream file(connectionStringFile);
        if (!file.is_open())
            return false;

        std::string connectionString;
        std::getline(file, connectionString);
        file.close();

        switch (SQLDriverConnect(sqlConnHandle, NULL, (SQLWCHAR*)std::wstring(connectionString.begin(), connectionString.end()).c_str(), SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT)) {
        case SQL_SUCCESS:
        case SQL_SUCCESS_WITH_INFO:
            return true;
        default:
            return false;
        }
    }

    void executeQuery(const std::string& queryFile, int column, const std::string& minDate, const std::string& maxDate) {
        std::ifstream file(queryFile);
        if (!file.is_open()) {
            std::cerr << "Failed to open query file: " << queryFile << std::endl;
            return;
        }

        std::ostringstream queryStream;
        queryStream << file.rdbuf();
        std::string query = queryStream.str();
        file.close();

        // Replace placeholders with actual dates
        size_t pos = query.find("<<MinDate>>");
        if (pos != std::string::npos) {
            query.replace(pos, 11, minDate);
        }
        pos = query.find("<<MaxDate>>");
        if (pos != std::string::npos) {
            query.replace(pos, 11, maxDate);
        }

        // Debug output to verify the query
        std::cout << "Executing query:\n" << query << std::endl;

        SQLCHAR sqlVersion[SQL_RESULT_LEN];
        SQLBIGINT ptrSqlVersion;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle)) {
            std::cerr << "Failed to allocate statement handle." << std::endl;
            return;
        }

        if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLWCHAR*)std::wstring(query.begin(), query.end()).c_str(), SQL_NTS)) {
            std::cerr << "Failed to execute query." << std::endl;
            return;
        }

        std::cout << "\nРезультат запроса:\n";
        int row = 1;
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            std::cout.width(4);
            std::cout << row++ << ": ";
            for (int i = 1; i <= column; ++i) {
                SQLGetData(sqlStmtHandle, i, SQL_CHAR, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);
                std::cout.width(60);
                std::cout << sqlVersion << ' ';
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    setlocale(LC_ALL, "rus");

    Database db;

    // Попытка подключения к базе данных
    if (!db.connect("connection.txt")) {
        std::cerr << "Failed to connect to the database." << std::endl;
        return 1;
    }
    std::cout << "Успешно подключено к SQL Server\n";

    // Форматирование дат  YYYY DD MM
    std::string minDate = "2024-13-01 00:00:00"; // Пример даты
    std::string maxDate = "2024-13-01 23:59:59"; // Пример даты

    // Выполнение запроса
    db.executeQuery("query.sql", 9, minDate, maxDate);

    // Пауза окна консоли - выход при нажатии любой кнопки
    std::cout << "\n Нажмите любую кнопку для завершения...";
    getchar();

    return 0;
}