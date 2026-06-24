#pragma once

#include <string>
#include <vector>

/**
 * @brief Represents a solid menu item.
 */
struct Food {
    std::string name;  /// @brief The name of the food
    int base_price;    /// @brief How much it is
};

/**
 * @brief Represents a liquid menu item.
 */
struct Beverage {
    std::string name;  /// @brief The name of the beverage
    int base_price;    /// @brief How much it is
};

/**
 * @brief Represents the food item in inventory.
 */
struct FoodEntry {
    int id;                    /// @brief To index our product entry
    Food product;              /// @brief What kind of Food it is?
    int current_stock = 0;     /// @brief How many we have in store?
    bool is_archived = false;  /// @brief Is it soft-deleted?
};

/**
 * @brief Represents the beverage item in inventory.
 */
struct BeverageEntry {
    int id;                    /// @brief To index our product entry
    Beverage product;          /// @brief What kind of Beverage it is?
    int current_stock = 0;     /// @brief How many we have in store?
    bool is_archived = false;  /// @brief Is it soft-deleted?
};

/**
 * @brief Represents a single line of a transaction.
 */
struct Order {
    std::string
        ticket;    /// @brief Customer name or ticket grouping related orders
    int entry_id;  /// @brief What product they ordered? (food or beverage id)
    int amount;    /// @brief How much they ordered?
};

/**
 * @brief Our inventory. Foods and beverages are kept in their own arrays
 *        so each can be looked up, saved, and loaded independently.
 */
struct Inventory {
    int next_id = 1;                       /// @brief The next id to use
    std::vector<FoodEntry> foods;          /// @brief Our food list
    std::vector<BeverageEntry> beverages;  /// @brief Our beverage list
    std::vector<Order> orders;             /// @brief Orders awaiting a receipt
};

// ===== Display Methods =====

/**
 * @brief Show a padded message with a new line.
 */
void show_message(int spacing, const std::string& message,
                  const std::string& color = "");

void show_title(const std::string& title);
void show_question(const std::string& prompt);
void show_option(const std::string& option);

void show_info(const std::string& message);
void show_error(const std::string& message);
void show_success(const std::string& message);

/**
 * @brief Render headers and rows as a bordered table.
 * @param headers Column headers, in display order
 * @param rows    Each row's cells, in the same column order as headers
 * @note Column widths fit their content automatically (so a table never
 *       prints wider than it needs to), but the whole table is capped
 *       at 65 characters wide — columns shrink, and their cells get
 *       truncated with "...", just enough to stay under that cap.
 */
void show_table(const std::vector<std::string>& headers,
                const std::vector<std::vector<std::string>>& rows);

/**
 * @brief Convert 'Food' to displayable string
 */
std::string food_to_string(const Food& food);

/**
 * @brief Convert 'Beverage' to displayable string
 */
std::string beverage_to_string(const Beverage& beverage);

/**
 * @brief Display a formatted food entry
 */
void display_food_entry(const FoodEntry& food_entry);

/**
 * @brief Display a formatted beverage entry
 */
void display_beverage_entry(const BeverageEntry& beverage_entry);

// ===== Entry Methods =====

/**
 * @brief Generate a new id for an entry product to use
 * @param inventory The inventory to generate an id to
 */
int generate_id(Inventory& inventory);

/**
 * @brief Create a new entry and add it to inventory
 * @param inventory The inventory to add entry to
 */
void create_entry(Inventory& inventory);

/**
 * @brief Soft-delete an entry from the inventory (sets is_archived). If
 *        it has pending orders, the user is warned that removing it
 *        will cancel them, and only proceeds if they confirm
 * @param inventory The inventory to remove from
 */
void remove_entry(Inventory& inventory);

/**
 * @brief Update an entry in the inventory. Name and price always apply.
 *        If the new stock would drop below what's already reserved by
 *        pending orders, the user is warned and, only if they confirm,
 *        those pending orders get canceled
 * @param inventory The inventory with the id
 */
void update_entry(Inventory& inventory);

/**
 * @brief Search for an entry in the inventory
 * @param inventory The inventory to search from
 */
void search_entry(Inventory& inventory);

// ===== Inventory Methods =====

/**
 * @brief Display each product's stock as a table: Product,
 *        Current_Stock, Order_Amount (pending, not-yet-receipted
 *        orders), and Stock_After_Ordering
 * @param inventory The inventory to view
 */
void view_stocks(Inventory& inventory);

/**
 * @brief View the product entries in detail
 * @param inventory The product inventory
 */
void view_inventory(Inventory& inventory);

/**
 * @brief Save the foods, beverages, and orders into their own CSV files
 * @param inventory The inventory content to save
 */
void save_inventory(Inventory& inventory);

/**
 * @brief Load the foods, beverages, and orders from their CSV files
 * @param inventory The inventory to populate
 */
void load_inventory(Inventory& inventory);

// ===== Order Methods =====

/**
 * @brief Reserve an amount of a product as a pending order under a
 *        customer/ticket name (asked first). Stock isn't deducted yet
 *        that happens when `generate_receipt` is confirmed
 */
void create_order(Inventory& inventory);

/**
 * @brief Ask for a customer/ticket first, then show only that ticket's
 *        pending orders and their running total
 */
void view_orders(Inventory& inventory);

/**
 * @brief Ask for a customer/ticket first, show only that ticket's
 *        pending orders, then cancel one of them by its listed number.
 *        Refuses if the chosen number doesn't belong to that ticket
 */
void remove_order(Inventory& inventory);

/**
 * @brief Ask for a customer/ticket first, show only that ticket's
 *        pending orders, then only if the user confirms, deduct
 *        their stock, remove just that ticket's orders, and log the
 *        receipt as generated. Other tickets' pending orders are
 *        untouched
 */
void generate_receipt(Inventory& inventory);
