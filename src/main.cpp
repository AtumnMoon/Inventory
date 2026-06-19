#include <iostream>

#include "inventory.hpp"
#include "utils.hpp"

int main() {
    Inventory inventory;
    load_inventory(inventory);

    bool running = true;

    while (running) {
        std::cout << '\n'
                  << "+---------------------------+\n"
                  << "+    Inventory System <3    +\n"
                  << "+---------------------------+\n";

        show_question("What do you want to do?");
        show_option("[1]: Create Entry");
        show_option("[2]: Remove Entry");
        show_option("[3]: Update Entry");
        show_option("[4]: Search Entry");
        show_option("[5]: View Stocks");
        show_option("[6]: View Inventory");
        show_option("[7]: Create Order");
        show_option("[8]: Generate Receipt");
        show_option("[9]: Save Inventory");
        show_option("[0]: Exit");

        const int choice = get_uint("Answer > ");

        switch (choice) {
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
            case 5:
                view_stocks(inventory);
                break;
            case 6:
                view_inventory(inventory);
                break;
            case 7:
                create_order(inventory);
                break;
            case 8:
                generate_reciept(inventory);
                break;
            case 9:
                save_inventory(inventory);
                break;
            case 0:
                running = false;
                break;
            default:
                show_message(0, "Invalid option. Try again.");
                break;
        }
    }

    save_inventory(inventory);
    return 0;
}
