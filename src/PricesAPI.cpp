//
// Created by silav on 12-11-2020.
//

#include "common.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <wx/string.h> // NOLINT(modernize-deprecated-headers)
#include "PricesAPI.h"

using json = nlohmann::json;

std::string PricesAPI::get_item_json_by_id(int32_t item_id) {
    auto url = wxString::Format("https://api.guildwars2.com/v2/items/%d", item_id);
    auto r = cpr::Get(cpr::Url{url}, cpr::VerifySsl(false));
    return r.text;
}

std::vector<int32_t> PricesAPI::get_item_id_listings(int32_t item_id) {
    auto r = cpr::Get(cpr::Url{"https://api.guildwars2.com//v2/commerce/listings"}, cpr::VerifySsl(false));
    auto listings_data = json::parse(r.text);
    std::vector<int32_t> ret;

    if (listings_data.is_array()) {
        for (auto &el: listings_data) {
            if (el.is_number_integer()) {
                ret.push_back(el);
            }
        }
    } else {
        throw UnexpectedDataReceived();
    }

    return ret;
}

template <class T>
ItemListings listing_from_json(T item_data) {
    if (item_data.is_object()) {
        ItemListings ret{item_data["id"]};

        for (auto &el: item_data["buys"]) {
            ret.buy_listings.push_back({
                   el["unit_price"],
                   el["quantity"]
           });
        }

        for (auto &el: item_data["sells"]) {
            ret.sell_listings.push_back({
                    el["unit_price"],
                    el["quantity"]
            });
        }

        return ret;
    } else {
        throw UnexpectedDataReceived();
    }
}

ItemListings PricesAPI::get_listings_by_id(int32_t item_id) {
    auto url = wxString::Format("https://api.guildwars2.com/v2/commerce/listings?ids=%d", item_id);
    auto r = cpr::Get(cpr::Url{url}, cpr::VerifySsl(false));
    auto listings_data = json::parse(r.text);

    return listing_from_json(listings_data[0]);
}

wxString join(const std::vector<int32_t> &list) {
    if (list.size() == 1) {
        return wxString::Format("%d", list[0]);
    } else if (list.size() > 1) {
        auto join = wxString::Format("%d", list[0]);
        auto result = std::accumulate(list.begin() + 1, list.end(), join,
                        [](auto val, auto next) {
            return val + wxString::Format(",%d", next);
        });

        return result;
    }

    return "";
}

std::vector<ItemListings> PricesAPI::get_listings_by_id_list(const std::vector<int32_t> &item_id_list) {
    std::vector<ItemListings> ret;

    if (item_id_list.empty()) {
        return ret; /* skip the rest */
    }

    auto joined = join(item_id_list);
    auto url = wxString::Format("https://api.guildwars2.com/v2/commerce/listings?ids=%s", joined);
    auto r = cpr::Get(cpr::Url{url}, cpr::VerifySsl(false));
    auto listings_data = json::parse(r.text);

    if (listings_data.is_array()) {
        for (auto& el: listings_data) {
            ret.emplace_back(listing_from_json(el));
        }
    } else {
        throw UnexpectedDataReceived();
    }

    return ret;
}

