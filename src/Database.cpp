#include <sqlite3.h>
#include <wx/wx.h>
#include "common.h"
#include "Database.h"

const char *db_def =
        "create table if not exists item ("
            "id INTEGER PRIMARY KEY,"
            "data text"
        ");"
        "create table if not exists historical_listing ("
            "item_id INTEGER,"
            "price INTEGER," /* in coins */
            "supply INTEGER,"
            "is_buy_listing INTEGER,"
            "time_added TEXT," /* when was this added?? */
            "FOREIGN KEY (item_id) REFERENCES item(id)"
        ");"
        "create table if not exists listing ("
            "item_id INTEGER,"
            "price INTEGER,"
            "supply INTEGER,"
            "is_buy_listing INTEGER,"
            "FOREIGN KEY (item_id) REFERENCES item(id)"
        ");"
        "CREATE TRIGGER IF NOT EXISTS create_historical AFTER DELETE "
        "ON listing "
        "BEGIN "
        "   INSERT INTO historical_listing(item_id, price, supply, is_buy_listing, time_added) VALUES "
        "   (OLD.item_id, OLD.price, OLD.supply, OLD.is_buy_listing, DATE('now'));"
        "END;";

const char* sql_insert_listing =
        "INSERT INTO listing (item_id, price, supply, is_buy_listing) VALUES "
        "(?1, ?2, ?3, ?4);";

const char* sql_insert_item =
        "INSERT INTO item(id, data) VALUES "
        "(?1, ?2);";

const char* sql_remove_item_listings =
        "DELETE FROM listing WHERE item_id=?1 AND is_buy_listing=?2";

const char* sql_count_items =
        "SELECT COUNT(*) FROM item WHERE id=?1";

const char* sql_get_item_name =
        "SELECT json_extract(data, '$.name') FROM item WHERE id=?1";

const char* sql_get_item_list =
        "SELECT id FROM item";

struct SqliteError : std::exception { int err; explicit SqliteError(int _err) { err = _err; } };
struct SqliteBindError : SqliteError { explicit SqliteBindError(int _err) : SqliteError(_err) {} };
struct SqliteRunError : SqliteError { explicit SqliteRunError(int _err) : SqliteError(_err) {} };

struct SqliteResult {
    sqlite3_stmt* stmt;
    bool has_data;
    SqliteResult(SqliteResult& other) = delete;
    ~SqliteResult() {
        sqlite3_reset(stmt);
    }

    int get_int(int colindex) {
        return sqlite3_column_int(stmt, colindex);
    }

    std::string get_text(int colindex) {
        const char *txt = (const char*)sqlite3_column_text(stmt, colindex);
        return std::string(txt);
    }

    bool fetch_next_row() {
        int res = sqlite3_step(stmt);
        if (res == SQLITE_ROW) {
            has_data = true;
        }
        else {
            has_data = false;
        }
        return has_data;
    }
};

class Database::DbImpl {
    sqlite3 *db{};

    void prepare_statements() {
        // stub
    }

    template<class A, class B, class... Ts>
    void prepare_statements(A& out, const B& str, Ts&... arg3) {
        if (db == nullptr)
            throw SqliteError(0);

        int err = sqlite3_prepare_v2(db, str, strlen(str), &out, nullptr);
        if (err != SQLITE_OK) {
            wxLogError(wxString::Format(
                    "Error preparing statement: '%s': %d: %s\n",
                    str,
                    err,
                    sqlite3_errmsg(db)
            ));
        }
        prepare_statements(arg3...);
    }

    void bind(sqlite3_stmt* unused) {
        // stub
    }

    void release_statement() {}

    template <class... Ts>
    void release_statement(sqlite3_stmt *&stmt, Ts&... rest) {
        sqlite3_finalize(stmt);
        stmt = nullptr;
        release_statement(rest...);
    }

    void create_database() {
        char *err = new char[2048];
        int err_int = sqlite3_exec(db, db_def, nullptr, nullptr, &err);
        if (err_int != SQLITE_OK) {
            wxLogError(wxString::Format("Error creating database: '%s': %d\n", err, err_int));
        }

        delete[] err;
    }

    void prepare_db_statements() {
        try {
            prepare_statements(
                    stmt_insert_listing, sql_insert_listing,
                    stmt_insert_item, sql_insert_item,
                    stmt_delete_item_listings, sql_remove_item_listings,
                    stmt_count_items, sql_count_items,
                    stmt_get_item_name, sql_get_item_name,
                    stmt_get_item_list, sql_get_item_list
            );
        } catch (SqliteError &err) {
            wxLogError(wxString::Format("Sqlite 3 error: %d", err.err));
        }
    }

    template<class... Ts>
    void bind(sqlite3_stmt *stmt, int index, int val, Ts... rest) {
        int err = sqlite3_bind_int(stmt, index, val);

        if (err != SQLITE_OK)
            throw SqliteBindError(err);

        bind(stmt, rest...);
    }

    template<class... Ts>
    void bind(sqlite3_stmt *stmt, int index, const char* val, Ts... rest) {
        int err = sqlite3_bind_text(stmt, index, val, strlen(val), nullptr);

        if (err != SQLITE_OK)
            throw SqliteBindError(err);

        bind(stmt, rest...);
    }

public:


    explicit DbImpl(wxString path) {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            wxLogError(wxString::Format("Couldn't open %s", path));
            db = nullptr;
            return;
        }

        create_database();
        prepare_db_statements();
    }

    ~DbImpl() {
        if (db) {
            sqlite3_close(db);
            release_statement(
                stmt_insert_listing,
                stmt_insert_item,
                stmt_delete_item_listings,
                stmt_count_items,
                stmt_get_item_name,
                stmt_get_item_list
            );
        }
    }

    sqlite3_stmt *stmt_insert_listing;
    sqlite3_stmt *stmt_insert_item;
    sqlite3_stmt *stmt_delete_item_listings;
    sqlite3_stmt *stmt_count_items;
    sqlite3_stmt *stmt_get_item_name;
    sqlite3_stmt *stmt_get_item_list;

    template<class... Ts>
    void run(sqlite3_stmt* stmt, Ts... rest) {
        bind(stmt, rest...);

        int step_err = sqlite3_step(stmt);
        if (step_err != SQLITE_OK) {
            if (step_err == SQLITE_DONE) {
                int reset_err = sqlite3_reset(stmt);
                if (reset_err != SQLITE_OK)
                    throw SqliteRunError(reset_err);
            } else
                throw SqliteRunError(step_err);
        }
    }

    template<class... Ts>
    SqliteResult query(sqlite3_stmt* stmt, Ts... rest) {
        bind(stmt, rest...);

        bool has_data = true;
        int step_err = sqlite3_step(stmt);
        if (step_err != SQLITE_ROW) {
            if (step_err == SQLITE_DONE) {
                has_data = false;
            } else 
                throw SqliteRunError(step_err);
        }

        return {
            stmt, has_data
        };
    }
};

void Database::open(wxString path) {
    impl = std::make_unique<DbImpl>(path.char_str());
}

void Database::add_listing(int32_t item_id, const std::vector<Listing> &listings, ListingType type) {
    /* remove listings for this item with this type */
    impl->run(
            impl->stmt_delete_item_listings,
            1, item_id,
            2, (int)type
    );

    /* add the new ones */
    for (auto listing: listings) {
        impl->run(
                impl->stmt_insert_listing,
                1, item_id,
                2, listing.price,
                3, listing.supply,
                4, (int)type
        );
    }
}

void Database::add_item(int32_t item_id, const std::string& json_str) {
    if (!item_exists(item_id)) {
        impl->run(
                impl->stmt_insert_item,
                1, item_id,
                2, json_str.c_str()
        );
    }
}

bool Database::item_exists(int32_t item_id) {
    auto result = impl->query(
            impl->stmt_count_items,
            1, item_id
    );

    return result.get_int(0) != 0;
}

Database::Database() {

}

Database::~Database() {
    /* keep the compiler happy because that is how pimpl do. */
}

std::string Database::get_item_name(long i) {
    auto result = impl->query(
            impl->stmt_get_item_name,
            1, i
    );

    return result.get_text(0);
}

std::vector<int32_t> Database::get_items_in_database() {
    auto result = impl->query(impl->stmt_get_item_list);
    std::vector<int32_t> ret;

    if (!result.has_data)
        return ret;

    /* fetch IDs */
    while (result.has_data) {
        ret.push_back(result.get_int(0));

        result.fetch_next_row();
    }

    return ret;
}

