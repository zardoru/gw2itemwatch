#include <algorithm>
#include <numeric>
#include <limits>
#include "common.h"

double ItemListings::get_price_max(bool use_buy_listings) {
    auto listing_max = [](int32_t val, const Listing& nval) {
        return std::max(val, nval.price);
    };

    if (use_buy_listings) {
        return std::reduce(buy_listings.begin(), buy_listings.end(), 0, listing_max);
    } else {
        return std::reduce(sell_listings.begin(), sell_listings.end(), 0, listing_max);
    }
}

double ItemListings::get_price_min(bool use_buy_listings) {
    auto listing_min = [](int32_t val, const Listing& nval) {
        return std::min(val, nval.price);
    };

    if (use_buy_listings) {
        return std::reduce(buy_listings.begin(), buy_listings.end(), std::numeric_limits<int32_t>::max(), listing_min);
    } else {
        return std::reduce(sell_listings.begin(), sell_listings.end(), std::numeric_limits<int32_t>::max(), listing_min);
    }
}

double ItemListings::get_price_avg(bool use_buy_listings) {
    auto listing_sum = [](double val, const Listing& nval) {
        return val + nval.price;
    };
    if (use_buy_listings) {
        return std::accumulate(buy_listings.begin(), buy_listings.end(), 0.0, listing_sum) / (double)buy_listings.size();
    } else {
        return std::accumulate(sell_listings.begin(), sell_listings.end(), 0.0, listing_sum) / (double)sell_listings.size();
    }
}
