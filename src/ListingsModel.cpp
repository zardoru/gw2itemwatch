#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-static-cast-downcast"

#include <wx/wx.h>
#include "common.h"
#include "ListingsModel.h"

ListingsModel::ListingsModel() = default;

ListingsModel::~ListingsModel() = default;

unsigned int ListingsModel::GetColumnCount() const {
    return (unsigned int)ListingColumns::COLUMN_MAX; /* see ListingColumns */
}

wxString format_coin(unsigned int coin) {
    auto copper = coin % 100;
    auto silver = (coin % 10000) / 100;
    auto gold = (coin % 1000000) / 10000;

    if (gold == 0) {
        if (silver == 0) {
            return wxString::Format("%d", copper);
        } else {
            return wxString::Format("%02d.%02d", silver, copper);
        }
    } else
        return wxString::Format("%02d.%02d.%02d", gold, silver, copper);
}

wxString ListingsModel::GetColumnType(unsigned int column) const {
    switch ((ListingColumns)column) {
        case ListingColumns::COLUMN_TITLE:
            return "string"; // item name
        case ListingColumns::COLUMN_ITEM_ID:
            return "long"; // item id
        case ListingColumns::COLUMN_SUPPLY: // NOLINT(bugprone-branch-clone)
            return "long"; // supply
        case ListingColumns::COLUMN_UNIT_PRICE:
            return "long"; // unit price
        case ListingColumns::COLUMN_TOTAL:
            return "string"; // supply * unit price
        default:
            return "null"; /* out of range? */
    }
}

void ListingsModel::GetValue(wxVariant &val, const wxDataViewItem &item, unsigned int col) const {
    if (!item.IsOk())
        return;

    auto node = (Node*) item.GetID();
    BuySellNode *buy_sell_node;
    ListingNode *listing_node;
    ItemNode *item_node;

    switch(node->get_type()) {
        case NodeType::NODE_ITEM:
            item_node = static_cast<ItemNode*>(node);
            /* if (col == (unsigned int)ListingColumns::COLUMN_NAME) {
                val = item_node->get_name();
            } else if (col == (unsigned int)ListingColumns::COLUMN_ITEM_ID) {
                val = (long)item_node->get_data()->item_id;
            }*/
            val = wxString::Format("%s (ID: %d) [Max Buying: %s | Min Selling: %s]",
                                   item_node->get_name(), item_node->get_id(),
                                   format_coin(item_node->get_data()->get_price_max(true)),
                                   format_coin(item_node->get_data()->get_price_min(false))
            );
            break;
        case NodeType::NODE_BUY_SELL:
            buy_sell_node = static_cast<BuySellNode*>(node);
            {
                auto min = buy_sell_node->get_parent()->get_data()->get_price_min(buy_sell_node->is_buy);
                auto max = buy_sell_node->get_parent()->get_data()->get_price_max(buy_sell_node->is_buy);
                auto avg = buy_sell_node->get_parent()->get_data()->get_price_avg(buy_sell_node->is_buy);
                val = wxString::Format(
                        "%s (%zu offerings) [%s-%s] (%s avg) Price: %s",
                        buy_sell_node->get_title(),
                        buy_sell_node->children.size(),
                        format_coin((unsigned int)round(min)),
                        format_coin((unsigned int)round(max)),
                        format_coin((unsigned int)round(avg)),
                        format_coin(buy_sell_node->is_buy ? min : max)
                );
            }
            break;
        case NodeType::NODE_ENTRY:
            listing_node = static_cast<ListingNode*>(node);
            get_listing_value(val, listing_node, col);
            break;
    }
}

bool ListingsModel::SetValue(const wxVariant &val, const wxDataViewItem &item, unsigned int column) {
    return false;
}

wxDataViewItem ListingsModel::GetParent(const wxDataViewItem &item) const {
    if (!item.IsOk())
        return wxDataViewItem(nullptr);

    auto node = (Node*) item.GetID();
    BuySellNode *buy_sell_node;
    ListingNode *listing_node;
    switch (node->get_type()) {
        case NodeType::NODE_ITEM:
            return wxDataViewItem(nullptr);
        case NodeType::NODE_BUY_SELL:
            buy_sell_node = static_cast<BuySellNode*>(node);
            return wxDataViewItem(buy_sell_node->get_parent());
        case NodeType::NODE_ENTRY:
            listing_node = static_cast<ListingNode*>(node);
            return wxDataViewItem(listing_node->get_parent());
    }

    /* unreachable hopefully */
    return wxDataViewItem(nullptr);
}

bool ListingsModel::IsContainer(const wxDataViewItem &item) const {
    if (!item.IsOk())
        return true; /* root node */

    auto node = (const Node*) item.GetID();
    switch (node->get_type()) {
        case NodeType::NODE_ITEM:
        case NodeType::NODE_BUY_SELL:
            return true;
        case NodeType::NODE_ENTRY:
            return false;
    }

    /* unreachable hopefully */
    return false;
}

unsigned ListingsModel::GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const {
    if (!item.IsOk()) {
        for (auto &child: root.children) {
            children.push_back(wxDataViewItem(child));
        }

        return root.children.size();
    }

    auto node = (Node*) item.GetID();
    ItemNode* item_node;
    BuySellNode* buy_sell_node;

    switch (node->get_type()) {
        case NodeType::NODE_ROOT:
            for (auto &child: root.children) {
                children.push_back(wxDataViewItem(child));
            }

            return root.children.size();
        case NodeType::NODE_ITEM:
            item_node = static_cast<ItemNode*>(node);
            children.push_back(wxDataViewItem(item_node->get_buy_node()));
            children.push_back(wxDataViewItem(item_node->get_sell_node()));
            return 2;
        case NodeType::NODE_ENTRY:
            return 0; // no children
        case NodeType::NODE_BUY_SELL:
            buy_sell_node = static_cast<BuySellNode*>(node);

            for (auto &child: buy_sell_node->children) {
                children.push_back(wxDataViewItem(&child));
            }

            return buy_sell_node->children.size();

    }

    /* unreachable hopefully */
    return 0;
}

void ListingsModel::get_listing_value(wxVariant &variant, ListingNode *pNode, unsigned int col) {
    switch ((ListingColumns)col) {
        case ListingColumns::COLUMN_TITLE:
            variant = wxString::Format(
                    "%s: %s",
                    pNode->get_parent()->get_title(),
                    pNode->get_parent()->get_parent()->get_name()
            );
        case ListingColumns::COLUMN_ITEM_ID:
            /* explicit conversion to avoid ambiguity */
            variant = (long)pNode->get_parent()->get_parent()->get_data()->item_id;
            break;
        case ListingColumns::COLUMN_SUPPLY:
            variant = (long)pNode->listing.supply;
            break;
        case ListingColumns::COLUMN_UNIT_PRICE:
            variant = format_coin(pNode->listing.price);
            break;
        case ListingColumns::COLUMN_TOTAL:
            variant = format_coin(pNode->listing.price * pNode->listing.supply);
        default:
            break;
    }
}

void ListingsModel::add_listing(const ItemListings &listing) {
    auto item_node = ItemNode::create_from_item_listing(&root, listing);
    root.children.push_back(item_node);
    ItemAdded(wxDataViewItem(nullptr), wxDataViewItem(item_node));
}

void ListingsModel::copy_listings(const std::vector<ItemListings> &listing) {
    wxDataViewItemArray added_ptrs;

    for (auto &l: listing) {
        auto item_node = ItemNode::create_from_item_listing(&root, l);
        root.children.push_back(item_node);
        added_ptrs.push_back(wxDataViewItem(item_node));
    }

    ItemsAdded(wxDataViewItem(nullptr), added_ptrs);
}

void ListingsModel::RemoveAllItemListings() {
    wxDataViewItemArray removed_ptrs;
    removed_ptrs.reserve(root.children.size());

    for (auto &l: root.children) {
        removed_ptrs.push_back(wxDataViewItem(l));
        delete l;
    }

    root.children.clear();

    ItemsDeleted(wxDataViewItem(nullptr), removed_ptrs);
}

ItemNode::ItemNode(RootNode* _parent) :
    Node(NodeType::NODE_ITEM),
    parent(_parent),
    buy_node(nullptr),
    sell_node(nullptr) {}

ItemNode* ItemNode::create_from_item_listing(RootNode* parent, const ItemListings &list) {
    auto node = new ItemNode(parent);
    node->data = list;

    node->buy_node = new BuySellNode(node);
    node->buy_node->is_buy = true;
    for (auto it: list.buy_listings) {
        auto new_node = ListingNode(node->buy_node, it);
        node->buy_node->children.push_back(new_node);
    }

    node->sell_node = new BuySellNode(node);
    node->sell_node->is_buy = false;
    for (auto it: list.sell_listings) {
        auto new_node = ListingNode(node->buy_node, it);
        node->sell_node->children.push_back(new_node);
    }

    return node;
}

BuySellNode *ItemNode::get_buy_node() {
    return buy_node;
}

BuySellNode *ItemNode::get_sell_node() {
    return sell_node;
}

ItemListings *ItemNode::get_data() {
    return &data;
}

wxString ItemNode::get_name() {
    return data.item_name;
}

RootNode *ItemNode::get_parent() {
    return parent;
}

ItemNode::~ItemNode() {
    delete buy_node;
    delete sell_node;
}

int ItemNode::get_id() {
    return data.item_id;
}

BuySellNode::BuySellNode(ItemNode *_parent) :
    Node(NodeType::NODE_BUY_SELL), parent(_parent) {}



wxString BuySellNode::get_title() const {
    if (is_buy)
        return _("Buying");
    else
        return _("Selling");
}

ItemNode *BuySellNode::get_parent() {
    return parent;
}

ListingNode::ListingNode(BuySellNode *_parent, Listing l) :
    Node(NodeType::NODE_ENTRY), parent(_parent), listing(l) {}

BuySellNode *ListingNode::get_parent() {
    return parent;
}

Node::Node(NodeType _t) : t(_t) {}

NodeType Node::get_type() const {
    return t;
}

#pragma clang diagnostic pop

RootNode::RootNode() : Node(NodeType::NODE_ROOT) {

}

RootNode::~RootNode() {
    for (auto ptr: children) {
        delete ptr;
    }
}
