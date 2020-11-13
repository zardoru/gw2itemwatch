#ifndef GW2ITEMWATCH_DATABASE_H
#define GW2ITEMWATCH_DATABASE_H


class Database {
    class DbImpl;
    std::unique_ptr<DbImpl> impl;
public:
    Database();
    ~Database();
    void open(wxString path);
    void add_listing(int32_t item_id, const std::vector<Listing> &l, ListingType type);
    void add_item(int32_t item_id, const std::string& json_str);
    bool item_exists(int32_t item_id);

    std::vector<int32_t> get_items_in_database();

    std::string get_item_name(long i);
};


#endif //GW2ITEMWATCH_DATABASE_H
