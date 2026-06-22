#include <cstddef>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "color.hpp"
#include "inventory.hpp"
#include "log.hpp"
#include "utils.hpp"

namespace {

// File names used by save_inventory / load_inventory.
// Foods, beverages, and orders each get their own CSV file.
const std::string FOODS_FILE = "foods.csv";
const std::string BEVERAGES_FILE = "beverages.csv";
const std::string ORDERS_FILE = "orders.csv";
const std::string CSV_DELIMITER = ",";

// Centralized here so changing a look later only means editing one line.
const std::string TITLE_COLOR = Color::Cyan + Color::Bold + Color::Highlight;
const std::string QUESTION_COLOR = Color::Cyan + Color::Bold;

// The hard cap show_table is allowed to print a line at — no row, no
// matter how much content it holds, may end up wider than this.
const std::size_t MAX_TABLE_WIDTH = 65;

// Cuts text down to fit a column. Adds "..." to mark the cut when
// there's room for it, so it stays obvious something got hidden instead
// of silently dropping characters.
std::string fit_cell(const std::string& text, std::size_t width) {
    if (text.size() <= width) {
        return text;
    }

    if (width <= 3) {
        return text.substr(0, width);
    }

    return text.substr(0, width - 3) + "...";
}

// Shrinks whichever column is currently widest, one character at a
// time, until the whole row fits in max_width. Greedy, but a table only
// ever has a handful of columns, so the loop stays cheap.
std::vector<std::size_t> fit_column_widths(std::vector<std::size_t> widths,
                                           std::size_t max_width) {
    // Every column costs a leading "|" plus 2 padding spaces, and the
    // row gets one extra trailing "|".
    const std::size_t overhead = widths.size() * 3 + 1;

    while (true) {
        std::size_t total = overhead;
        for (std::size_t width : widths) {
            total += width;
        }

        if (total <= max_width) {
            break;
        }

        std::size_t widest = 0;
        for (std::size_t i = 1; i < widths.size(); ++i) {
            if (widths[i] > widths[widest]) {
                widest = i;
            }
        }

        // Stop once a column is too small to stay readable — the row
        // may end up slightly over budget, but that beats a useless one.
        if (widths[widest] <= 3) {
            break;
        }

        widths[widest] -= 1;
    }

    return widths;
}

// Lays out one row as "| cell | cell | ...". std::format's width spec
// ({:<{}}) does the left-align-and-pad in one call instead of manually
// building padding strings.
std::string format_table_row(const std::vector<std::string>& cells,
                             const std::vector<std::size_t>& widths) {
    std::string line = "|";

    for (std::size_t i = 0; i < widths.size(); ++i) {
        const std::string cell = (i < cells.size()) ? cells[i] : "";
        const std::string fitted = fit_cell(cell, widths[i]);
        line += " " + std::format("{:<{}}", fitted, widths[i]) + " |";
    }

    return line;
}

// Helper functions for looking up indexes.
// "Not found" is signaled by returning the vector's own size — the same
// idiom std::string::npos uses, and a value no real index can ever equal.

std::size_t find_food_index(const Inventory& inventory, int id) {
    for (std::size_t i = 0; i < inventory.foods.size(); ++i) {
        if (inventory.foods[i].id == id) {
            return i;
        }
    }
    return inventory.foods.size();
}

std::size_t find_beverage_index(const Inventory& inventory, int id) {
    for (std::size_t i = 0; i < inventory.beverages.size(); ++i) {
        if (inventory.beverages[i].id == id) {
            return i;
        }
    }
    return inventory.beverages.size();
}

// Prints each pending order's line item and returns the running total.
// Shared by view_orders (just looking) and generate_reciept (about to
// commit), so the two can never drift out of sync with each other.
int print_order_lines(const Inventory& inventory) {
    int total = 0;
    const std::string item_color = Color::Green;

    for (const auto& order : inventory.orders) {
        const std::size_t food_index =
            find_food_index(inventory, order.entry_id);
        if (food_index != inventory.foods.size()) {
            const FoodEntry& entry = inventory.foods[food_index];
            const int subtotal = entry.product.base_price * order.amount;
            total += subtotal;
            show_message(2,
                         entry.product.name + " x" +
                             std::to_string(order.amount) + " = " +
                             std::to_string(subtotal),
                         item_color);
            continue;
        }

        const std::size_t beverage_index =
            find_beverage_index(inventory, order.entry_id);
        if (beverage_index != inventory.beverages.size()) {
            const BeverageEntry& entry = inventory.beverages[beverage_index];
            const int subtotal = entry.product.base_price * order.amount;
            total += subtotal;
            show_message(2,
                         entry.product.name + " x" +
                             std::to_string(order.amount) + " = " +
                             std::to_string(subtotal),
                         item_color);
        }
    }

    return total;
}

// Sums every pending (not-yet-receipted) order amount for one product —
// used by view_stocks to show how much of an item is already reserved.
int sum_pending_orders(const Inventory& inventory, int entry_id) {
    int total = 0;
    for (const auto& order : inventory.orders) {
        if (order.entry_id == entry_id) {
            total += order.amount;
        }
    }
    return total;
}

// Builds one product's row for the stocks table: name, current stock,
// how much of it sits in pending orders, and what stock will be once
// those orders' receipt gets generated.
std::vector<std::string> build_stock_row(const Inventory& inventory,
                                         const std::string& name, int id,
                                         int current_stock) {
    const int order_amount = sum_pending_orders(inventory, id);

    // NOTE: create_order deducts stock the moment an order is placed,
    // not when its receipt gets generated — so under the current
    // design there's nothing left to subtract here yet. See the reply
    // text for why that makes this column a flat copy of current_stock
    // for now.
    const int stock_after_ordering = current_stock;

    return std::vector<std::string>{name, std::to_string(current_stock),
                                    std::to_string(order_amount),
                                    std::to_string(stock_after_ordering)};
}

}  // namespace

// ===== Display Methods =====

void show_message(int spacing, const std::string& message,
                  const std::string& color) {
    const std::string padding = std::string(spacing, ' ');

    if (!color.empty()) {
        std::cout << color;
    }

    std::cout << padding << message;

    if (!color.empty()) {
        std::cout << Color::Reset;
    }

    std::cout << '\n';
}

void show_title(const std::string& title) {
    const std::string message = "[ " + title + " ]";
    show_message(0, message, TITLE_COLOR);
}

void show_question(const std::string& prompt) {
    show_message(2, prompt, QUESTION_COLOR);
}

void show_option(const std::string& option) {
    show_message(4, option, Color::Blue);
}

void show_info(const std::string& message) {
    show_message(0, message, Color::White);
}

void show_error(const std::string& message) {
    show_message(0, message, Color::Red);
}

void show_success(const std::string& message) {
    show_message(0, message, Color::Green);
}

void show_table(const std::vector<std::string>& headers,
                const std::vector<std::vector<std::string>>& rows) {
    if (headers.empty()) {
        return;
    }

    // Every column starts as wide as its header, then grows to fit
    std::vector<std::size_t> widths;
    for (const auto& header : headers) {
        widths.push_back(header.size());
    }
    for (const auto& row : rows) {
        for (std::size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            if (row[i].size() > widths[i]) {
                widths[i] = row[i].size();
            }
        }
    }

    // Only shrinks and starts truncating once the natural size would
    // have overflowed the 65-character cap.
    widths = fit_column_widths(widths, MAX_TABLE_WIDTH);

    // The "+----+----+" border line
    std::string border = "+";
    for (std::size_t width : widths) {
        border += std::string(width + 2, '-') + "+";
    }

    show_message(0, border);
    show_message(0, format_table_row(headers, widths));
    show_message(0, border);
    for (const auto& row : rows) {
        show_message(0, format_table_row(row, widths));
    }
    show_message(0, border);
}

std::string food_to_string(const Food& food) {
    std::string result;
    result = "name: " + food.name + ", ";
    result += "price: " + std::to_string(food.base_price);
    return result;
}

std::string beverage_to_string(const Beverage& beverage) {
    std::string result;
    result = "name: " + beverage.name + ", ";
    result += "price: " + std::to_string(beverage.base_price);
    return result;
}

void display_food_entry(const FoodEntry& food_entry) {
    const std::string title_color = Color::Yellow + Color::Bold;
    const std::string content_color = Color::Magenta;

    show_message(0, "Food #" + std::to_string(food_entry.id), title_color);
    show_message(2, "Name: " + food_entry.product.name, content_color);
    show_message(2, "Price: " + std::to_string(food_entry.product.base_price),
                 content_color);
    show_message(2, "Stock: " + std::to_string(food_entry.current_stock),
                 content_color);
}

void display_beverage_entry(const BeverageEntry& beverage_entry) {
    const std::string title_color = Color::Yellow + Color::Bold;
    const std::string content_color = Color::Magenta;

    show_message(0, "Beverage #" + std::to_string(beverage_entry.id),
                 title_color);
    show_message(2, "Name: " + beverage_entry.product.name, content_color);
    show_message(2,
                 "Price: " + std::to_string(beverage_entry.product.base_price),
                 content_color);
    show_message(2, "Stock: " + std::to_string(beverage_entry.current_stock),
                 content_color);
}

// ===== Entry Methods =====

int generate_id(Inventory& inventory) { return inventory.next_id++; }

void create_entry(Inventory& inventory) {
    // Prompt the user
    show_question("What product do you want?");
    show_option("[1]: Food");
    show_option("[2]: Beverage");
    show_option("[0]: Exit");

    // Get the answer
    const int choice = get_uint("Answer > ");

    // The user picked Food
    if (choice == 1) {
        // Ask for data required to create 'Food'
        const std::string name = get_string("Name: ");
        const int base_price = get_uint("Price: ");
        const int current_stock = get_uint("Amount: ");

        // Create the 'Food'
        const Food food{name, base_price};

        // Ask for confirmation to add in inventory
        const bool confirmation =
            prompt_yes_no("Add \"Food\" to inventory? (Y/n): ");
        if (confirmation) {
            // Les go!~
            // Let's create an inventory entry
            // to track our Food in inventory
            const int id = generate_id(inventory);
            const FoodEntry e{id, food, current_stock, false};

            // Now we add it to inventory
            inventory.foods.push_back(e);

            // Add it to logs
            const std::string component = "id=" + std::to_string(e.id);
            const std::string message =
                "Successfully added " + e.product.name + " to food inventory!";
            save_log(create_log("SUCCESS", component, message));
        }
    } else if (choice == 2) {
        // Ask for data required to create 'Beverage'
        const std::string name = get_string("Name: ");
        const int base_price = get_uint("Price: ");
        const int current_stock = get_uint("Amount: ");

        // Create the 'Beverage'
        const Beverage beverage{name, base_price};

        // Ask for confirmation to add in inventory
        const bool confirmation =
            prompt_yes_no("Add \"Beverage\" to inventory? (Y/n): ");
        if (confirmation) {
            // Les go!~
            // Let's create an inventory entry
            // to track our Beverage in inventory
            const int id = generate_id(inventory);
            const BeverageEntry e{id, beverage, current_stock, false};

            // Now we add it to inventory
            inventory.beverages.push_back(e);

            // Add it to logs
            const std::string component = "id=" + std::to_string(e.id);
            const std::string message = "Successfully added " + e.product.name +
                                        " to beverage inventory!";
            save_log(create_log("SUCCESS", component, message));
        }
    }

    // Exit
    return;
}

void remove_entry(Inventory& inventory) {
    show_question("Remove which product type?");
    show_option("[1]: Food");
    show_option("[2]: Beverage");
    show_option("[0]: Exit");

    const int choice = get_uint("Answer > ");
    if (choice != 1 && choice != 2) {
        return;
    }

    // Get the entry id
    const int id = get_uint("Entry id: ");

    // They chose food, run this
    if (choice == 1) {
        // Find the index of the food
        const std::size_t index = find_food_index(inventory, id);
        if (index == inventory.foods.size() ||
            inventory.foods[index].is_archived) {
            show_error("No matching food entry found.");
            return;
        }

        FoodEntry& entry = inventory.foods[index];

        // Ask for confirmation
        const bool confirmation =
            prompt_yes_no("Remove \"" + entry.product.name + "\"? (Y/n): ");
        if (confirmation) {
            // Then delete it
            entry.is_archived = true;

            const std::string component = "id=" + std::to_string(entry.id);
            const std::string message =
                "Removed " + entry.product.name + " from food inventory!";
            save_log(create_log("SUCCESS", component, message));
        }
    } else {
        // They chose beverage, find the index of beverage
        const std::size_t index = find_beverage_index(inventory, id);
        if (index == inventory.beverages.size() ||
            inventory.beverages[index].is_archived) {
            show_error("No matching beverage entry found.");
            return;
        }

        BeverageEntry& entry = inventory.beverages[index];

        // Ask for confirmation
        const bool confirmation =
            prompt_yes_no("Remove \"" + entry.product.name + "\"? (Y/n): ");
        if (confirmation) {
            // Then delete it
            entry.is_archived = true;

            const std::string component = "id=" + std::to_string(entry.id);
            const std::string message =
                "Removed " + entry.product.name + " from beverage inventory!";
            save_log(create_log("SUCCESS", component, message));
        }
    }
}

void update_entry(Inventory& inventory) {
    show_question("Update which product type?");
    show_option("[1]: Food");
    show_option("[2]: Beverage");
    show_option("[0]: Exit");

    const int choice = get_uint("Answer > ");
    if (choice != 1 && choice != 2) {
        return;
    }

    const int id = get_uint("Entry id: ");

    if (choice == 1) {
        const std::size_t index = find_food_index(inventory, id);
        if (index == inventory.foods.size() ||
            inventory.foods[index].is_archived) {
            show_error("No matching food entry found.");
            return;
        }

        FoodEntry& entry = inventory.foods[index];

        show_info("Leave blank to keep the current value.");
        entry.product.name = get_string_or_skip(
            "Name [" + entry.product.name + "]: ", entry.product.name);
        entry.product.base_price = get_uint_or_skip(
            "Price [" + std::to_string(entry.product.base_price) + "]: ",
            entry.product.base_price);
        entry.current_stock = get_uint_or_skip(
            "Stock [" + std::to_string(entry.current_stock) + "]: ",
            entry.current_stock);

        const std::string component = "id=" + std::to_string(entry.id);
        const std::string message =
            "Updated " + entry.product.name + " in food inventory!";
        save_log(create_log("SUCCESS", component, message));
    } else {
        const std::size_t index = find_beverage_index(inventory, id);
        if (index == inventory.beverages.size() ||
            inventory.beverages[index].is_archived) {
            show_error("No matching beverage entry found.");
            return;
        }

        BeverageEntry& entry = inventory.beverages[index];

        show_info("Leave blank to keep the current value.");
        entry.product.name = get_string_or_skip(
            "Name [" + entry.product.name + "]: ", entry.product.name);
        entry.product.base_price = get_uint_or_skip(
            "Price [" + std::to_string(entry.product.base_price) + "]: ",
            entry.product.base_price);
        entry.current_stock = get_uint_or_skip(
            "Stock [" + std::to_string(entry.current_stock) + "]: ",
            entry.current_stock);

        const std::string component = "id=" + std::to_string(entry.id);
        const std::string message =
            "Updated " + entry.product.name + " in beverage inventory!";
        save_log(create_log("SUCCESS", component, message));
    }
}

void search_entry(Inventory& inventory) {
    show_question("Search which product type?");
    show_option("[1]: Food");
    show_option("[2]: Beverage");
    show_option("[0]: Exit");

    const int choice = get_uint("Answer > ");
    if (choice != 1 && choice != 2) {
        return;
    }

    const int id = get_uint("Entry id: ");

    if (choice == 1) {
        const std::size_t index = find_food_index(inventory, id);
        if (index == inventory.foods.size()) {
            show_error("No matching food entry found.");
            return;
        }
        display_food_entry(inventory.foods[index]);
    } else {
        const std::size_t index = find_beverage_index(inventory, id);
        if (index == inventory.beverages.size()) {
            show_error("No matching beverage entry found.");
            return;
        }
        display_beverage_entry(inventory.beverages[index]);
    }
}

// ===== Inventory Methods =====

void view_stocks(Inventory& inventory) {
    show_title("Stocks");

    const std::string sub_title_color =
        std::string(Color::Yellow) + Color::Bold;
    const std::vector<std::string> headers = {
        "Product", "Current_Stock", "Order_Amount", "Stock_After_Ordering"};

    // Collect food rows first so we know whether there's anything to
    // render before deciding between a table and an empty-state message.
    std::vector<std::vector<std::string>> food_rows;
    for (const auto& entry : inventory.foods) {
        // Skip "deleted" foods
        if (entry.is_archived) {
            continue;
        }
        food_rows.push_back(build_stock_row(inventory, entry.product.name,
                                            entry.id, entry.current_stock));
    }

    show_message(2, "Foods", sub_title_color);
    if (food_rows.empty()) {
        show_info("No foods in stock.");
    } else {
        show_table(headers, food_rows);
    }

    show_message(0, "");  // Blank line between sections

    std::vector<std::vector<std::string>> beverage_rows;
    for (const auto& entry : inventory.beverages) {
        // Skip "deleted" beverages
        if (entry.is_archived) {
            continue;
        }
        beverage_rows.push_back(build_stock_row(inventory, entry.product.name,
                                                entry.id, entry.current_stock));
    }

    show_message(2, "Beverages", sub_title_color);
    if (beverage_rows.empty()) {
        show_info("No beverages in stock.");
    } else {
        show_table(headers, beverage_rows);
    }
}

void view_inventory(Inventory& inventory) {
    show_title("Foods");
    for (const auto& entry : inventory.foods) {
        display_food_entry(entry);
    }

    std::cout << '\n';  // Add new line

    show_title("Beverages");
    for (const auto& entry : inventory.beverages) {
        display_beverage_entry(entry);
    }
}

void save_inventory(Inventory& inventory) {
    // Foods -> foods.csv
    std::ofstream foods_file(FOODS_FILE);
    if (!foods_file.is_open()) {
        show_error("Failed to open \"" + FOODS_FILE + "\" for writing.");
    } else {
        foods_file << "id,name,base_price,current_stock,is_archived\n";
        for (const auto& entry : inventory.foods) {
            foods_file << entry.id << CSV_DELIMITER << entry.product.name
                       << CSV_DELIMITER << entry.product.base_price
                       << CSV_DELIMITER << entry.current_stock << CSV_DELIMITER
                       << (entry.is_archived ? 1 : 0) << '\n';
        }
        foods_file.close();
    }

    // Beverages -> beverages.csv
    std::ofstream beverages_file(BEVERAGES_FILE);
    if (!beverages_file.is_open()) {
        show_error("Failed to open \"" + BEVERAGES_FILE + "\" for writing.");
    } else {
        beverages_file << "id,name,base_price,current_stock,is_archived\n";
        for (const auto& entry : inventory.beverages) {
            beverages_file << entry.id << CSV_DELIMITER << entry.product.name
                           << CSV_DELIMITER << entry.product.base_price
                           << CSV_DELIMITER << entry.current_stock
                           << CSV_DELIMITER << (entry.is_archived ? 1 : 0)
                           << '\n';
        }
        beverages_file.close();
    }

    // Orders -> orders.csv
    std::ofstream orders_file(ORDERS_FILE);
    if (!orders_file.is_open()) {
        show_error("Failed to open \"" + ORDERS_FILE + "\" for writing.");
    } else {
        orders_file << "entry_id,amount\n";
        for (const auto& order : inventory.orders) {
            orders_file << order.entry_id << CSV_DELIMITER << order.amount
                        << '\n';
        }
        orders_file.close();
    }

    save_log(create_log("SUCCESS", "save_inventory",
                        "Inventory saved to CSV files."));
    show_success("Inventory saved.");
}

void load_inventory(Inventory& inventory) {
    inventory.foods.clear();
    inventory.beverages.clear();
    inventory.orders.clear();

    int max_id = 0;

    // Foods <- foods.csv
    std::ifstream foods_file(FOODS_FILE);
    if (foods_file.is_open()) {
        std::string line;
        std::getline(foods_file, line);  // skip header

        while (std::getline(foods_file, line)) {
            if (line.empty()) {
                continue;
            }

            const std::vector<std::string> fields = split(line, CSV_DELIMITER);
            if (fields.size() < 5) {
                continue;
            }

            FoodEntry entry;
            entry.id = std::stoi(fields[0]);
            entry.product.name = fields[1];
            entry.product.base_price = std::stoi(fields[2]);
            entry.current_stock = std::stoi(fields[3]);
            entry.is_archived = (fields[4] == "1");

            inventory.foods.push_back(entry);
            if (entry.id > max_id) {
                max_id = entry.id;
            }
        }
        foods_file.close();
    }

    // Beverages <- beverages.csv
    std::ifstream beverages_file(BEVERAGES_FILE);
    if (beverages_file.is_open()) {
        std::string line;
        std::getline(beverages_file, line);  // skip header

        while (std::getline(beverages_file, line)) {
            if (line.empty()) {
                continue;
            }

            const std::vector<std::string> fields = split(line, CSV_DELIMITER);
            if (fields.size() < 5) {
                continue;
            }

            BeverageEntry entry;
            entry.id = std::stoi(fields[0]);
            entry.product.name = fields[1];
            entry.product.base_price = std::stoi(fields[2]);
            entry.current_stock = std::stoi(fields[3]);
            entry.is_archived = (fields[4] == "1");

            inventory.beverages.push_back(entry);
            if (entry.id > max_id) {
                max_id = entry.id;
            }
        }
        beverages_file.close();
    }

    // Orders <- orders.csv
    std::ifstream orders_file(ORDERS_FILE);
    if (orders_file.is_open()) {
        std::string line;
        std::getline(orders_file, line);  // skip header

        while (std::getline(orders_file, line)) {
            if (line.empty()) {
                continue;
            }

            const std::vector<std::string> fields = split(line, CSV_DELIMITER);
            if (fields.size() < 2) {
                continue;
            }

            Order order;
            order.entry_id = std::stoi(fields[0]);
            order.amount = std::stoi(fields[1]);

            inventory.orders.push_back(order);
        }
        orders_file.close();
    }

    inventory.next_id = max_id + 1;

    save_log(create_log("SUCCESS", "load_inventory",
                        "Inventory loaded from CSV files."));
}

// ===== Order Methods =====

void create_order(Inventory& inventory) {
    show_question("Order which product type?");
    show_option("[1]: Food");
    show_option("[2]: Beverage");
    show_option("[0]: Exit");

    const int choice = get_uint("Answer > ");
    if (choice != 1 && choice != 2) {
        return;
    }

    const int id = get_uint("Entry id: ");

    if (choice == 1) {
        const std::size_t index = find_food_index(inventory, id);
        if (index == inventory.foods.size() ||
            inventory.foods[index].is_archived) {
            show_error("No matching food entry found.");
            return;
        }

        FoodEntry& entry = inventory.foods[index];
        const int amount = get_uint("Amount: ");

        if (amount > entry.current_stock) {
            show_error("Not enough stock available.");
            return;
        }

        entry.current_stock -= amount;
        inventory.orders.push_back(Order{entry.id, amount});

        const std::string component = "id=" + std::to_string(entry.id);
        const std::string message = "Ordered " + std::to_string(amount) +
                                    " of " + entry.product.name + ".";
        save_log(create_log("SUCCESS", component, message));
        show_success("Order placed.");
    } else {
        const std::size_t index = find_beverage_index(inventory, id);
        if (index == inventory.beverages.size() ||
            inventory.beverages[index].is_archived) {
            show_error("No matching beverage entry found.");
            return;
        }

        BeverageEntry& entry = inventory.beverages[index];
        const int amount = get_uint("Amount: ");

        if (amount > entry.current_stock) {
            show_error("Not enough stock available.");
            return;
        }

        entry.current_stock -= amount;
        inventory.orders.push_back(Order{entry.id, amount});

        const std::string component = "id=" + std::to_string(entry.id);
        const std::string message = "Ordered " + std::to_string(amount) +
                                    " of " + entry.product.name + ".";
        save_log(create_log("SUCCESS", component, message));
        show_success("Order placed.");
    }
}

void view_orders(Inventory& inventory) {
    if (inventory.orders.empty()) {
        show_info("No pending orders.");
        return;
    }

    show_title("Pending Orders");

    const int total = print_order_lines(inventory);
    show_message(0, "Total: " + std::to_string(total), Color::Magenta);
}

void generate_reciept(Inventory& inventory) {
    // Show what's pending first
    view_orders(inventory);

    if (inventory.orders.empty()) {
        return;
    }

    const bool confirmation =
        prompt_yes_no("Generate receipt for these orders? (Y/n): ");
    if (!confirmation) {
        return;
    }

    inventory.orders.clear();
    save_log(create_log("SUCCESS", "generate_reciept", "Receipt generated."));
    show_success("Receipt generated.");
}
