//
// Created by silav on 12-11-2020.
//

#ifndef GW2ITEMWATCH_PRICESAPI_H
#define GW2ITEMWATCH_PRICESAPI_H

class PricesAPI {
public:
    static std::string get_item_json_by_id(int32_t item_id);
    static std::vector<int32_t> get_item_id_listings(int32_t item_id);
    static ItemListings get_listings_by_id(int32_t item_id);
    static std::vector<ItemListings> get_listings_by_id_list(const std::vector<int32_t> &item_id_list);
};

class UnexpectedDataReceived : std::exception {};

#endif //GW2ITEMWATCH_PRICESAPI_H
