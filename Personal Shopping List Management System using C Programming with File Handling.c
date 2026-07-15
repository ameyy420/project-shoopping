#include <stdio.h>
#include <string.h>

#define MAX_ITEMS 100

typedef struct {
    char item_name[50];
    char category[30];
    int desired_quantity;
    int variant_quantities[2];
} ShoppingItem;

ShoppingItem items[MAX_ITEMS];
int totalItems = 0;

const char *filename = "shopping.txt";


void loadRecords() {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("No saved records found (shopping.txt missing).\n");
        return;
    }

    totalItems = 0;
    while (fscanf(fp, "%49[^,],%29[^,],%d,%d,%d\n",
                  items[totalItems].item_name,
                  items[totalItems].category,
                  &items[totalItems].desired_quantity,
                  &items[totalItems].variant_quantities[0],
                  &items[totalItems].variant_quantities[1]) == 5) {
        totalItems++;
        if (totalItems >= MAX_ITEMS) break;
    }

    fclose(fp);
    printf("Records loaded successfully. (%d items)\n", totalItems);
}


void saveRecords() {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Error: Unable to open file for writing.\n");
        return;
    }

    for (int i = 0; i < totalItems; i++) {
        fprintf(fp, "%s,%s,%d,%d,%d\n",
                items[i].item_name,
                items[i].category,
                items[i].desired_quantity,
                items[i].variant_quantities[0],
                items[i].variant_quantities[1]);
    }

    fclose(fp);
    printf("Records saved successfully.\n");
}


void addNewItem() {
    if (totalItems >= MAX_ITEMS) {
        printf("Error: Maximum item limit reached.\n");
        return;
    }

    ShoppingItem newItem;

    getchar();
    printf("Enter Item Name: ");
    fgets(newItem.item_name, sizeof(newItem.item_name), stdin);
    newItem.item_name[strcspn(newItem.item_name, "\n")] = 0;

    printf("Enter Category: ");
    fgets(newItem.category, sizeof(newItem.category), stdin);
    newItem.category[strcspn(newItem.category, "\n")] = 0;

    printf("Enter Desired Quantity: ");
    scanf("%d", &newItem.desired_quantity);

    printf("Enter Variant 1 Quantity (0 if N/A): ");
    scanf("%d", &newItem.variant_quantities[0]);

    printf("Enter Variant 2 Quantity (0 if N/A): ");
    scanf("%d", &newItem.variant_quantities[1]);

    items[totalItems++] = newItem;
    printf("Item added successfully.\n");
}


void viewAllItems() {
    if (totalItems == 0) {
        printf("No items to display.\n");
        return;
    }

    printf("\nShopping List:\n");
    for (int i = 0; i < totalItems; i++) {
        printf("%d. %s | Category: %s | Desired: %d | Variant1: %d | Variant2: %d\n",
               i + 1,
               items[i].item_name,
               items[i].category,
               items[i].desired_quantity,
               items[i].variant_quantities[0],
               items[i].variant_quantities[1]);
    }
}


void searchItem() {
    char searchName[50];
    int found = 0;

    getchar();
    printf("Enter Item Name to search: ");
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = 0;

    for (int i = 0; i < totalItems; i++) {
        if (strcmp(items[i].item_name, searchName) == 0) {
            printf("Item Found:\n");
            printf("%s | Category: %s | Desired: %d | Variant1: %d | Variant2: %d\n",
                   items[i].item_name,
                   items[i].category,
                   items[i].desired_quantity,
                   items[i].variant_quantities[0],
                   items[i].variant_quantities[1]);
            found = 1;
        }
    }

    if (!found) {
        printf("Item not found.\n");
    }
}


void countItemsByCategory() {
    char searchCategory[30];
    int count = 0;

    getchar();
    printf("Enter Category to count: ");
    fgets(searchCategory, sizeof(searchCategory), stdin);
    searchCategory[strcspn(searchCategory, "\n")] = 0;

    for (int i = 0; i < totalItems; i++) {
        if (strcmp(items[i].category, searchCategory) == 0) {
            count++;
        }
    }

    printf("Items in category '%s': %d\n", searchCategory, count);
}


void menu() {
    int choice = 0;

    for (;;) {
        printf("\nPersonal Shopping List \n");
        printf("1. Load Records\n");
        printf("2. Save Records\n");
        printf("3. Add New Item\n");
        printf("4. View All Items\n");
        printf("5. Search Item\n");
        printf("6. Count Items by Category\n");
        printf("7. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            loadRecords();
        } else if (choice == 2) {
            saveRecords();
        } else if (choice == 3) {
            addNewItem();
        } else if (choice == 4) {
            viewAllItems();
        } else if (choice == 5) {
            searchItem();
        } else if (choice == 6) {
            countItemsByCategory();
        } else if (choice == 7) {
            printf("Exiting program.\n");
            break;
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }
}

int main() {
    menu();
    return 0;
}
