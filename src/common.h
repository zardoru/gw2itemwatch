#ifndef GW2ITEMWATCH_COMMON_H
#define GW2ITEMWATCH_COMMON_H

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

enum class ListingType {
    SELL = 0,
    BUY = 1
};

struct Listing {
    int32_t price;
    int32_t supply; /* quantity in the api. listings count ignored*/
};

struct ItemListings {
    int32_t item_id;
    std::string item_name;
    std::vector<Listing> buy_listings;
    std::vector<Listing> sell_listings;

    double get_price_max(bool use_buy_listings);
    double get_price_min(bool use_buy_listings);
    double get_price_avg(bool use_buy_listings);
};



#endif //GW2ITEMWATCH_COMMON_H
