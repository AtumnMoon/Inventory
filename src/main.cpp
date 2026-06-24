#include <iostream>

#include "inventory.hpp"
#include "utils.hpp"

int main() {
    // Create and populate the inventory using the content from save file
    Inventory inventory;
    load_inventory(inventory);

    bool running = true;

    while (running) {
        std::cout << '\n'
                << "+---------------------------+\n"
                << "+    Inventory System <3    +\n"
                << "+---------------------------+\n";

        show_question("What do you want to do?");
        show_option("[1]: Manage Products");
        show_option("[2]: Manage Orders");
        show_option("[3]: View Stocks");
        show_option("[4]: View Inventory");
        show_option("[5]: Save Inventory");
        show_option("[0]: Exit");

        switch (get_uint("Answer > ")) {
            case 1: {
                // Entry-related actions, grouped behind their own
                // sub menu so the top-level menu stays short.
                bool in_submenu = true;
                while (in_submenu) {
                    show_title("Manage Products");
                    show_option("[1]: Create Products");
                    show_option("[2]: Remove Products");
                    show_option("[3]: Update Products");
                    show_option("[4]: Search Products");
                    show_option("[0]: Back");

                    switch (get_uint("Answer > ")) {
                        case 1:
                            create_entry(inventory);
                            break;
                        case 2:
                            remove_entry(inventory);
                            break;
                        case 3:
                            update_entry(inventory);
                            break;
                        case 4:
                            search_entry(inventory);
                            break;
                        case 0:
                            in_submenu = false;
                            break;
                        default:
                            show_error("Invalid option. Try again.");
                            break;
                    }
                }
                break;
            }
            case 2: {
                // Order-related actions, same grouping idea.
                bool in_submenu = true;
                while (in_submenu) {
                    show_title("Manage Orders");
                    show_option("[1]: Create Order");
                    show_option("[2]: View Orders");
                    show_option("[3]: Remove Order");
                    show_option("[4]: Generate Receipt");
                    show_option("[0]: Back");

                    switch (get_uint("Answer > ")) {
                        case 1:
                            create_order(inventory);
                            break;
                        case 2:
                            view_orders(inventory);
                            break;
                        case 3:
                            remove_order(inventory);
                            break;
                        case 4:
                            generate_receipt(inventory);
                            break;
                        case 0:
                            in_submenu = false;
                            break;
                        default:
                            show_error("Invalid option. Try again.");
                            break;
                    }
                }
                break;
            }
            case 3:
                view_stocks(inventory);
                break;
            case 4:
                view_inventory(inventory);
                break;
            case 5:
                save_inventory(inventory);
                break;
            case 0:
                running = false;
                break;
            default:
                show_error("Invalid option. Try again.");
                break;
        }
    }

    save_inventory(inventory);
    return 0;
}
