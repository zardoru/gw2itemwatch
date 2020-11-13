//
// Created by silav on 12-11-2020.
//

#ifndef GW2ITEMWATCH_LISTINGSMODEL_H
#define GW2ITEMWATCH_LISTINGSMODEL_H

#include <wx/dataview.h>

enum class NodeType {
    NODE_ITEM,
    NODE_BUY_SELL,
    NODE_ENTRY,
    NODE_ROOT
};

class Node {
protected:
    NodeType t;
    Node(NodeType _t);
public:
    NodeType get_type() const;
};

class BuySellNode;
class ItemNode;
class RootNode;

class ListingNode : public Node {
    BuySellNode* parent;
public:
    Listing listing;
    ListingNode(BuySellNode *_parent, Listing l);
    BuySellNode* get_parent();
};

class BuySellNode : public Node {
    ItemNode* parent;
public:
    std::vector<ListingNode> children;
    explicit BuySellNode(ItemNode* _parent);
    ItemNode* get_parent();
    wxString get_title() const;

    bool is_buy{};
};

class ItemNode : public Node {
    ItemListings data{};
    RootNode* parent;

    /* unique_ptr was doing some weird ass shit to my data */
    BuySellNode* buy_node;
    BuySellNode* sell_node;
public:
    ItemNode(RootNode* _parent);
    ~ItemNode();
    static ItemNode* create_from_item_listing(RootNode* parent, const ItemListings &list);

    wxString get_name();
    BuySellNode* get_buy_node();
    BuySellNode* get_sell_node();
    ItemListings* get_data();
    RootNode* get_parent();

    int get_id();
};

class RootNode : public Node {
public:
    std::vector<ItemNode*> children;
    RootNode();
    ~RootNode();
};

enum class ListingColumns : unsigned int {
    COLUMN_TITLE,
    COLUMN_ITEM_ID,
    COLUMN_SUPPLY,
    COLUMN_UNIT_PRICE,
    COLUMN_TOTAL,
    COLUMN_MAX
};

class ListingsModel : public wxDataViewModel {
    RootNode root;
public:
    ListingsModel();
    ~ListingsModel() override;
    unsigned int GetColumnCount() const override;
    wxString GetColumnType(unsigned int column) const override;
    void GetValue(wxVariant &val, const wxDataViewItem &item, unsigned int col) const override;
    bool SetValue(const wxVariant& val, const wxDataViewItem& item, unsigned int column) override;
    wxDataViewItem GetParent(const wxDataViewItem& item) const override;
    bool IsContainer(const wxDataViewItem& item) const override;
    unsigned GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const override;

    static void get_listing_value(wxVariant &variant, ListingNode *pNode, unsigned int col) ;

    /**
     * Copy item listing into the model.
     * Manages its own copy, so it's fine.
     * @param listing
     */
    void add_listing(const ItemListings &listing);

    /**
     * Copy item listings in the given vector
     * into the model.
     * Makes a copy of the data.
     * @param listing
     */
    void copy_listings(const std::vector<ItemListings> &listing);

    /**
     * Empty the data tree.
     */
    void RemoveAllItemListings();
};


#endif //GW2ITEMWATCH_LISTINGSMODEL_H
