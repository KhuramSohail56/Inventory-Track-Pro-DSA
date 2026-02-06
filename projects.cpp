#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>
#include <limits>
#include <cctype>
#include <algorithm>
using namespace std;

struct Product {
    string id;
    string name;
    string category;
    double price;
    double rating;
    int stock;
    int sales;
};

struct HashNode {
    Product product;
    HashNode* next;
};

struct BSTNode {
    Product product;
    BSTNode* left;
    BSTNode* right;
};

struct StackNode {
    string operation;
    Product product;
    Product oldProduct;
    StackNode* next;
};

HashNode* hashTable[100];
BSTNode* bstRoot;
StackNode* undoStack = NULL;
StackNode* redoStack = NULL;
Product productArray[1000];
int productCount = 0;

// ========== INPUT VALIDATION FUNCTIONS ==========
bool isValidDoubleInput(const string& input) {
    if (input.empty()) return false;
    
    bool dotFound = false;
    for (size_t i = 0; i < input.length(); i++) {
        if (i == 0 && input[i] == '-') continue; // Allow negative
        if (input[i] == '.') {
            if (dotFound) return false; // Multiple dots
            dotFound = true;
        } else if (!isdigit(input[i])) {
            return false;
        }
    }
    return true;
}

bool isValidIntInput(const string& input) {
    if (input.empty()) return false;
    
    for (size_t i = 0; i < input.length(); i++) {
        if (i == 0 && input[i] == '-') continue; // Allow negative
        if (!isdigit(input[i])) return false;
    }
    return true;
}

bool isValidRating(double rating) {
    return rating >= 0.0 && rating <= 5.0;
}

bool isValidStock(int stock) {
    return stock >= 0;
}

bool isValidSales(int sales) {
    return sales >= 0;
}

bool isValidPrice(double price) {
    return price >= 0.0;
}

bool isValidProductID(const string& id) {
    if (id.empty()) return false;
    
    // Check if ID contains only alphanumeric characters
    for (char c : id) {
        if (!isalnum(c) && c != '-' && c != '_') {
            return false;
        }
    }
    return true;
}

bool isValidProductName(const string& name) {
    return !name.empty();
}

// ========== SAFE INPUT FUNCTIONS ==========
string getStringInput(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Remove leading/trailing whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (!input.empty()) {
            return input;
        }
        cout << "Error: Input cannot be empty. Please try again.\n";
    }
}

double getDoubleInput(const string& prompt, double min = -1e9, double max = 1e9) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Remove whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (input.empty()) {
            cout << "Error: Input cannot be empty. Please try again.\n";
            continue;
        }
        
        if (!isValidDoubleInput(input)) {
            cout << "Error: Invalid number format. Please enter a valid number.\n";
            continue;
        }
        
        try {
            double value = stod(input);
            if (value < min || value > max) {
                cout << "Error: Value must be between " << min << " and " << max << ". Please try again.\n";
                continue;
            }
            return value;
        } catch (const invalid_argument& e) {
            cout << "Error: Invalid number format. Please enter a valid number.\n";
        } catch (const out_of_range& e) {
            cout << "Error: Number is out of range. Please enter a smaller number.\n";
        }
    }
}

int getIntInput(const string& prompt, int min = -1e9, int max = 1e9) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Remove whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (input.empty()) {
            cout << "Error: Input cannot be empty. Please try again.\n";
            continue;
        }
        
        if (!isValidIntInput(input)) {
            cout << "Error: Invalid integer format. Please enter a whole number.\n";
            continue;
        }
        
        try {
            int value = stoi(input);
            if (value < min || value > max) {
                cout << "Error: Value must be between " << min << " and " << max << ". Please try again.\n";
                continue;
            }
            return value;
        } catch (const invalid_argument& e) {
            cout << "Error: Invalid integer format. Please enter a whole number.\n";
        } catch (const out_of_range& e) {
            cout << "Error: Number is out of range. Please enter a smaller number.\n";
        }
    }
}

int getMenuChoice(int minChoice, int maxChoice) {
    int choice;
    while (true) {
        cout << "Enter your choice (" << minChoice << "-" << maxChoice << "): ";
        cin >> choice;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: Invalid input. Please enter a number.\n";
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (choice >= minChoice && choice <= maxChoice) {
            return choice;
        }
        
        cout << "Error: Choice must be between " << minChoice << " and " << maxChoice << ". Please try again.\n";
    }
}

// ========== EXISTING FUNCTIONS WITH ERROR HANDLING ==========
int hashFunction(string id) {
    int hash = 0;
    for (int i = 0; i < id.length(); i++) {
        hash = hash * 31 + id[i];
    }
    if (hash < 0) hash = -hash;
    return hash % 100;
}

void pushStack(StackNode* &stack, string op, Product p, Product old) {
    StackNode* newNode = new StackNode;
    newNode->operation = op;
    newNode->product = p;
    newNode->oldProduct = old;
    newNode->next = stack;
    stack = newNode;
}

int popStack(StackNode* &stack, string &op, Product &p, Product &old) {
    if (stack == NULL) return 0;
    
    StackNode* temp = stack;
    op = temp->operation;
    p = temp->product;
    old = temp->oldProduct;
    stack = stack->next;
    delete temp;
    return 1;
}

BSTNode* createBSTNode(Product p) {
    BSTNode* newNode = new BSTNode;
    newNode->product = p;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

BSTNode* insertBST(BSTNode* root, Product p) {
    if (root == NULL) {
        return createBSTNode(p);
    }
    
    if (p.price < root->product.price) {
        root->left = insertBST(root->left, p);
    } else {
        root->right = insertBST(root->right, p);
    }
    return root;
}

BSTNode* findMinBST(BSTNode* root) {
    while (root != NULL && root->left != NULL) {
        root = root->left;
    }
    return root;
}

BSTNode* deleteBST(BSTNode* root, string id) {
    if (root == NULL) return root;
    
    if (root->product.id == id) {
        if (root->left == NULL) {
            BSTNode* temp = root->right;
            delete root;
            return temp;
        } else if (root->right == NULL) {
            BSTNode* temp = root->left;
            delete root;
            return temp;
        }
        
        BSTNode* temp = findMinBST(root->right);
        root->product = temp->product;
        root->right = deleteBST(root->right, temp->product.id);
    } else {
        root->left = deleteBST(root->left, id);
        root->right = deleteBST(root->right, id);
    }
    return root;
}

void rangeSearchBST(BSTNode* root, double minPrice, double maxPrice) {
    if (root == NULL) return;
    
    if (root->product.price > minPrice) {
        rangeSearchBST(root->left, minPrice, maxPrice);
    }
    
    if (root->product.price >= minPrice && root->product.price <= maxPrice) {
        cout << "ID: " << root->product.id << "\tName: " << root->product.name 
             << "\tPrice: $" << root->product.price << endl;
    }
    
    if (root->product.price < maxPrice) {
        rangeSearchBST(root->right, minPrice, maxPrice);
    }
}

void mergeProducts(Product arr[], int left, int mid, int right, int sortBy, bool ascending) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    Product* leftArr = new Product[n1];
    Product* rightArr = new Product[n2];
    
    for (int i = 0; i < n1; i++) leftArr[i] = arr[left + i];
    for (int i = 0; i < n2; i++) rightArr[i] = arr[mid + 1 + i];
    
    int i = 0, j = 0, k = left;
    
    while (i < n1 && j < n2) {
        bool shouldSwap;
        
        if (sortBy == 1) {
            if (ascending) {
                shouldSwap = leftArr[i].price <= rightArr[j].price;
            } else {
                shouldSwap = leftArr[i].price >= rightArr[j].price;
            }
        } else if (sortBy == 2) {
            if (ascending) {
                shouldSwap = leftArr[i].rating <= rightArr[j].rating;
            } else {
                shouldSwap = leftArr[i].rating >= rightArr[j].rating;
            }
        } else {
            if (ascending) {
                shouldSwap = leftArr[i].sales <= rightArr[j].sales;
            } else {
                shouldSwap = leftArr[i].sales >= rightArr[j].sales;
            }
        }
        
        if (shouldSwap) {
            arr[k++] = leftArr[i++];
        } else {
            arr[k++] = rightArr[j++];
        }
    }
    
    while (i < n1) arr[k++] = leftArr[i++];
    while (j < n2) arr[k++] = rightArr[j++];
    
    delete[] leftArr;
    delete[] rightArr;
}

void mergeSortProducts(Product arr[], int left, int right, int sortBy, bool ascending) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortProducts(arr, left, mid, sortBy, ascending);
        mergeSortProducts(arr, mid + 1, right, sortBy, ascending);
        mergeProducts(arr, left, mid, right, sortBy, ascending);
    }
}

void quickSortProducts(Product arr[], int low, int high, int sortBy, bool ascending) {
    if (low < high) {
        Product pivot = arr[high];
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            bool shouldSwap;
            
            if (sortBy == 1) {
                if (ascending) {
                    shouldSwap = arr[j].price <= pivot.price;
                } else {
                    shouldSwap = arr[j].price >= pivot.price;
                }
            } else if (sortBy == 2) {
                if (ascending) {
                    shouldSwap = arr[j].rating <= pivot.rating;
                } else {
                    shouldSwap = arr[j].rating >= pivot.rating;
                }
            } else {
                if (ascending) {
                    shouldSwap = arr[j].sales <= pivot.sales;
                } else {
                    shouldSwap = arr[j].sales >= pivot.sales;
                }
            }
            
            if (shouldSwap) {
                i++;
                Product temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        
        Product temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;
        
        int pi = i + 1;
        quickSortProducts(arr, low, pi - 1, sortBy, ascending);
        quickSortProducts(arr, pi + 1, high, sortBy, ascending);
    }
}

void copyProductsToArray() {
    productCount = 0;
    for (int i = 0; i < 100; i++) {
        HashNode* current = hashTable[i];
        while (current != NULL && productCount < 1000) {
            productArray[productCount] = current->product;
            productCount++;
            current = current->next;
        }
    }
}

void displayProducts(Product arr[], int n) {
    if (n == 0) {
        cout << "No products to display.\n";
        return;
    }
    
    cout << "\n" << left << setw(8) << "ID";
    cout << left << setw(25) << "Name";
    cout << left << setw(12) << "Price";
    cout << left << setw(8) << "Rating";
    cout << left << setw(10) << "Stock";
    cout << left << setw(8) << "Sales\n";
    cout << string(70, '-') << endl;
    for (int i = 0; i < n; i++) {
        cout << left << setw(8) << arr[i].id;
        cout << left << setw(25) << arr[i].name;
        cout << "$" << left << setw(11) << fixed << setprecision(2) << arr[i].price;
        cout << left << setw(8) << fixed << setprecision(1) << arr[i].rating;
        cout << left << setw(10) << arr[i].stock;
        cout << left << setw(8) << arr[i].sales << endl;
    }
    cout << string(70, '-') << endl;
}

void initSystem() {
    for (int i = 0; i < 100; i++) {
        hashTable[i] = NULL;
    }
    bstRoot = NULL;
    undoStack = NULL;
    redoStack = NULL;
    productCount = 0;
}

void addProduct(Product p) {
    if (!isValidProductID(p.id)) {
        cout << "Error: Invalid Product ID!\n";
        return;
    }
    
    if (!isValidProductName(p.name)) {
        cout << "Error: Product name cannot be empty!\n";
        return;
    }
    
    if (!isValidPrice(p.price)) {
        cout << "Error: Price must be non-negative!\n";
        return;
    }
    
    if (!isValidRating(p.rating)) {
        cout << "Error: Rating must be between 0.0 and 5.0!\n";
        return;
    }
    
    if (!isValidStock(p.stock)) {
        cout << "Error: Stock must be non-negative!\n";
        return;
    }
    
    if (!isValidSales(p.sales)) {
        cout << "Error: Sales must be non-negative!\n";
        return;
    }
    
    int index = hashFunction(p.id);
    
    HashNode* current = hashTable[index];
    while (current != NULL) {
        if (current->product.id == p.id) {
            cout << "Error: Product ID already exists!\n";
            return;
        }
        current = current->next;
    }
    
    HashNode* newNode = new HashNode;
    newNode->product = p;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
    
    bstRoot = insertBST(bstRoot, p);
    
    Product empty;
    empty.id = "";
    pushStack(undoStack, "ADD", p, empty);
    
    cout << "Product added successfully!\n";
}

Product* searchProduct(string id) {
    if (!isValidProductID(id)) {
        cout << "Error: Invalid Product ID format!\n";
        return NULL;
    }
    
    int index = hashFunction(id);
    HashNode* current = hashTable[index];
    
    while (current != NULL) {
        if (current->product.id == id) {
            return &current->product;
        }
        current = current->next;
    }
    return NULL;
}

void updateProduct(string id, Product newProduct) {
    if (!isValidProductID(id)) {
        cout << "Error: Invalid Product ID!\n";
        return;
    }
    
    Product* oldProduct = searchProduct(id);
    if (oldProduct == NULL) {
        cout << "Product not found!\n";
        return;
    }
    
    if (!isValidProductName(newProduct.name)) {
        cout << "Error: Product name cannot be empty!\n";
        return;
    }
    
    if (!isValidPrice(newProduct.price)) {
        cout << "Error: Price must be non-negative!\n";
        return;
    }
    
    if (!isValidRating(newProduct.rating)) {
        cout << "Error: Rating must be between 0.0 and 5.0!\n";
        return;
    }
    
    if (!isValidStock(newProduct.stock)) {
        cout << "Error: Stock must be non-negative!\n";
        return;
    }
    
    if (!isValidSales(newProduct.sales)) {
        cout << "Error: Sales must be non-negative!\n";
        return;
    }
    
    Product oldCopy = *oldProduct;
    pushStack(undoStack, "UPDATE", newProduct, oldCopy);
    
    *oldProduct = newProduct;
    
    bstRoot = deleteBST(bstRoot, id);
    bstRoot = insertBST(bstRoot, newProduct);
    
    cout << "Product updated successfully!\n";
}

void deleteProduct(string id) {
    if (!isValidProductID(id)) {
        cout << "Error: Invalid Product ID!\n";
        return;
    }
    
    int index = hashFunction(id);
    HashNode* current = hashTable[index];
    HashNode* prev = NULL;
    
    while (current != NULL) {
        if (current->product.id == id) {
            pushStack(undoStack, "DELETE", current->product, current->product);
            
            if (prev == NULL) {
                hashTable[index] = current->next;
            } else {
                prev->next = current->next;
            }
            
            bstRoot = deleteBST(bstRoot, id);
            delete current;
            
            cout << "Product deleted successfully!\n";
            return;
        }
        prev = current;
        current = current->next;
    }
    cout << "Product not found!\n";
}

void displayAllProducts() {
    cout << "\n=== ALL PRODUCTS ===\n";
    cout << left << setw(8) << "ID";
    cout << left << setw(25) << "Name";
    cout << left << setw(20) << "Category";
    cout << left << setw(12) << "Price";
    cout << left << setw(8) << "Rating";
    cout << left << setw(10) << "Stock";
    cout << left << setw(8) << "Sales" << endl;
    cout << string(90, '-') << endl;
    
    int count = 0;
    for (int i = 0; i < 100; i++) {
        HashNode* current = hashTable[i];
        while (current != NULL) {
            Product p = current->product;
            
            cout << left << setw(8) << p.id;
            cout << left << setw(25) << p.name;
            cout << left << setw(20) << p.category;
            cout << "$" << left << setw(11) << fixed << setprecision(2) << p.price;
            cout << left << setw(8) << fixed << setprecision(1) << p.rating;
            cout << left << setw(10) << p.stock;
            cout << left << setw(8) << p.sales << endl;
            
            count++;
            current = current->next;
        }
    }
    cout << string(90, '-') << endl;
    cout << "Total Products: " << count << "\n";
}

void sortProducts(int sortBy, int algorithm, bool ascending) {
    copyProductsToArray();
    
    if (productCount == 0) {
        cout << "No products to sort!\n";
        return;
    }
    
    if (algorithm == 1) {
        mergeSortProducts(productArray, 0, productCount - 1, sortBy, ascending);
        cout << "\nSorted using Merge Sort:\n";
    } else {
        quickSortProducts(productArray, 0, productCount - 1, sortBy, ascending);
        cout << "\nSorted using Quick Sort:\n";
    }
    
    displayProducts(productArray, productCount);
}

void rangeSearch(double minPrice, double maxPrice) {
    if (minPrice < 0 || maxPrice < 0) {
        cout << "Error: Prices cannot be negative!\n";
        return;
    }
    
    if (minPrice > maxPrice) {
        cout << "Error: Minimum price cannot be greater than maximum price!\n";
        return;
    }
    
    cout << "\n=== PRODUCTS IN PRICE RANGE $" << minPrice << " to $" << maxPrice << " ===\n";
    rangeSearchBST(bstRoot, minPrice, maxPrice);
}

void undoOperation() {
    string op;
    Product p, old;
    
    if (!popStack(undoStack, op, p, old)) {
        cout << "Nothing to undo!\n";
        return;
    }
    
    pushStack(redoStack, op, p, old);
    
    if (op == "ADD") {
        deleteProduct(p.id);
    } else if (op == "DELETE") {
        addProduct(p);
    } else if (op == "UPDATE") {
        updateProduct(p.id, old);
    }
    
    cout << "Undo operation completed!\n";
}

void redoOperation() {
    string op;
    Product p, old;
    
    if (!popStack(redoStack, op, p, old)) {
        cout << "Nothing to redo!\n";
        return;
    }
    
    if (op == "ADD") {
        addProduct(p);
    } else if (op == "DELETE") {
        deleteProduct(p.id);
    } else if (op == "UPDATE") {
        updateProduct(p.id, p);
    }
    
    cout << "Redo operation completed!\n";
}

void saveToFile(string filename) {
    if (filename.empty()) {
        cout << "Error: Filename cannot be empty!\n";
        return;
    }
    
    ofstream file(filename);
    if (!file) {
        cout << "Error: Could not open file '" << filename << "' for writing!\n";
        return;
    }
    
    int count = 0;
    for (int i = 0; i < 100; i++) {
        HashNode* current = hashTable[i];
        while (current != NULL) {
            Product p = current->product;
            file << p.id << "," << p.name << "," << p.category << ","
                 << p.price << "," << p.rating << "," 
                 << p.stock << "," << p.sales << "\n";
            count++;
            current = current->next;
        }
    }
    
    file.close();
    if (file.fail()) {
        cout << "Error: Failed to write to file!\n";
        return;
    }
    
    cout << "Successfully saved " << count << " products to " << filename << "\n";
}

void loadFromFile(string filename) {
    if (filename.empty()) {
        cout << "Error: Filename cannot be empty!\n";
        return;
    }
    
    ifstream file(filename);
    if (!file) {
        cout << "Error: Could not open file '" << filename << "' for reading!\n";
        return;
    }
    
    initSystem();
    
    string line;
    int count = 0;
    int lineNum = 0;
    
    while (getline(file, line)) {
        lineNum++;
        if (line.empty()) continue; // Skip empty lines
        
        Product p;
        vector<string> fields;
        string field = "";
        
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == ',') {
                fields.push_back(field);
                field = "";
            } else {
                field += line[i];
            }
        }
        fields.push_back(field);
        
        if (fields.size() < 7) {
            cout << "Warning: Line " << lineNum << " has insufficient fields. Skipping...\n";
            continue;
        }
        
        try {
            p.id = fields[0];
            p.name = fields[1];
            p.category = fields[2];
            p.price = stod(fields[3]);
            p.rating = stod(fields[4]);
            p.stock = stoi(fields[5]);
            p.sales = stoi(fields[6]);
            
            if (isValidProductID(p.id) && isValidProductName(p.name) && 
                isValidPrice(p.price) && isValidRating(p.rating) &&
                isValidStock(p.stock) && isValidSales(p.sales)) {
                addProduct(p);
                count++;
            } else {
                cout << "Warning: Line " << lineNum << " contains invalid data. Skipping...\n";
            }
        } catch (const invalid_argument& e) {
            cout << "Warning: Line " << lineNum << " contains invalid number format. Skipping...\n";
        } catch (const out_of_range& e) {
            cout << "Warning: Line " << lineNum << " contains number out of range. Skipping...\n";
        }
    }
    
    file.close();
    cout << "Successfully loaded " << count << " products from " << filename << "\n";
}

void displayMenu() {
    cout << "\n=== INVENTORY TRACK PRO ===\n";
    cout << "1. Add Product\n";
    cout << "2. Search Product\n";
    cout << "3. Update Product\n";
    cout << "4. Delete Product\n";
    cout << "5. Display All Products\n";
    cout << "6. Sort Products\n";
    cout << "7. Range Search by Price\n";
    cout << "8. Undo Last Operation\n";
    cout << "9. Redo Last Operation\n";
    cout << "10. Save to File\n";
    cout << "11. Load from File\n";
    cout << "12. Exit\n";
    cout << "============================\n";
}

void sortMenu() {
    cout << "\n=== SORT OPTIONS ===\n";
    cout << "Sort by:\n";
    cout << "1. Price\n";
    cout << "2. Rating\n";
    cout << "3. Sales\n";
}

void algorithmMenu() {
    cout << "\n=== SORTING ALGORITHM ===\n";
    cout << "1. Merge Sort\n";
    cout << "2. Quick Sort\n";
}

void orderMenu() {
    cout << "\n=== SORT ORDER ===\n";
    cout << "1. Ascending\n";
    cout << "2. Descending\n";
}

int main() {
    initSystem();
    
    cout << "========================================\n";
    cout << "   INVENTORY TRACK PRO - DSA PROJECT   \n";
    cout << "========================================\n";
    
    while (true) {
        displayMenu();
        int choice = getMenuChoice(1, 12);
        
        if (choice == 1) {
            Product p;
            cout << "\n=== ADD NEW PRODUCT ===\n";
            
            while (true) {
                p.id = getStringInput("Enter Product ID: ");
                if (isValidProductID(p.id)) break;
                cout << "Error: Product ID can only contain letters, numbers, hyphens, and underscores.\n";
            }
            
            p.name = getStringInput("Enter Product Name: ");
            p.category = getStringInput("Enter Category: ");
            p.price = getDoubleInput("Enter Price: $", 0.0, 1000000.0);
            p.rating = getDoubleInput("Enter Rating (0-5): ", 0.0, 5.0);
            p.stock = getIntInput("Enter Stock: ", 0, 1000000);
            p.sales = getIntInput("Enter Sales: ", 0, 1000000);
            
            addProduct(p);
        }
        else if (choice == 2) {
            cout << "\n=== SEARCH PRODUCT ===\n";
            string id = getStringInput("Enter Product ID to search: ");
            
            Product* p = searchProduct(id);
            if (p != NULL) {
                cout << "\nProduct Found:\n";
                cout << "ID: " << p->id << "\n";
                cout << "Name: " << p->name << "\n";
                cout << "Category: " << p->category << "\n";
                cout << "Price: $" << fixed << setprecision(2) << p->price << "\n";
                cout << "Rating: " << fixed << setprecision(1) << p->rating << "\n";
                cout << "Stock: " << p->stock << "\n";
                cout << "Sales: " << p->sales << "\n";
            } else {
                cout << "Product not found!\n";
            }
        }
        else if (choice == 3) {
            cout << "\n=== UPDATE PRODUCT ===\n";
            string id = getStringInput("Enter Product ID to update: ");
            
            Product* old = searchProduct(id);
            if (old != NULL) {
                Product newProduct = *old;
                
                cout << "\nCurrent Product Information:\n";
                cout << "Name: " << old->name << "\n";
                cout << "Category: " << old->category << "\n";
                cout << "Price: $" << fixed << setprecision(2) << old->price << "\n";
                cout << "Rating: " << fixed << setprecision(1) << old->rating << "\n";
                cout << "Stock: " << old->stock << "\n";
                cout << "Sales: " << old->sales << "\n";
                
                cout << "\nEnter new values (press Enter to keep current value):\n";
                
                string input = getStringInput("New Name: ");
                if (!input.empty()) newProduct.name = input;
                
                input = getStringInput("New Category: ");
                if (!input.empty()) newProduct.category = input;
                
                while (true) {
                    input = getStringInput("New Price (press Enter to skip): ");
                    if (input.empty()) break;
                    if (isValidDoubleInput(input)) {
                        double price = stod(input);
                        if (isValidPrice(price)) {
                            newProduct.price = price;
                            break;
                        } else {
                            cout << "Error: Price must be non-negative!\n";
                        }
                    } else {
                        cout << "Error: Invalid price format!\n";
                    }
                }
                
                while (true) {
                    input = getStringInput("New Rating (0-5, press Enter to skip): ");
                    if (input.empty()) break;
                    if (isValidDoubleInput(input)) {
                        double rating = stod(input);
                        if (isValidRating(rating)) {
                            newProduct.rating = rating;
                            break;
                        } else {
                            cout << "Error: Rating must be between 0.0 and 5.0!\n";
                        }
                    } else {
                        cout << "Error: Invalid rating format!\n";
                    }
                }
                
                while (true) {
                    input = getStringInput("New Stock (press Enter to skip): ");
                    if (input.empty()) break;
                    if (isValidIntInput(input)) {
                        int stock = stoi(input);
                        if (isValidStock(stock)) {
                            newProduct.stock = stock;
                            break;
                        } else {
                            cout << "Error: Stock must be non-negative!\n";
                        }
                    } else {
                        cout << "Error: Invalid stock format!\n";
                    }
                }
                
                while (true) {
                    input = getStringInput("New Sales (press Enter to skip): ");
                    if (input.empty()) break;
                    if (isValidIntInput(input)) {
                        int sales = stoi(input);
                        if (isValidSales(sales)) {
                            newProduct.sales = sales;
                            break;
                        } else {
                            cout << "Error: Sales must be non-negative!\n";
                        }
                    } else {
                        cout << "Error: Invalid sales format!\n";
                    }
                }
                
                updateProduct(id, newProduct);
            } else {
                cout << "Product not found!\n";
            }
        }
        else if (choice == 4) {
            cout << "\n=== DELETE PRODUCT ===\n";
            string id = getStringInput("Enter Product ID to delete: ");
            
            cout << "Are you sure you want to delete product '" << id << "'? (y/n): ";
            char confirm;
            cin >> confirm;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            if (confirm == 'y' || confirm == 'Y') {
                deleteProduct(id);
            } else {
                cout << "Deletion cancelled.\n";
            }
        }
        else if (choice == 5) {
            displayAllProducts();
        }
        else if (choice == 6) {
            cout << "\n=== SORT PRODUCTS ===\n";
            
            sortMenu();
            int sortBy = getMenuChoice(1, 3);
            
            algorithmMenu();
            int algorithm = getMenuChoice(1, 2);
            
            orderMenu();
            int order = getMenuChoice(1, 2);
            
            sortProducts(sortBy, algorithm, order == 1);
        }
        else if (choice == 7) {
            cout << "\n=== RANGE SEARCH BY PRICE ===\n";
            
            double minPrice = getDoubleInput("Enter minimum price: $", 0.0, 1000000.0);
            double maxPrice;
            
            while (true) {
                maxPrice = getDoubleInput("Enter maximum price: $", 0.0, 1000000.0);
                if (maxPrice >= minPrice) break;
                cout << "Error: Maximum price must be greater than or equal to minimum price!\n";
            }
            
            rangeSearch(minPrice, maxPrice);
        }
        else if (choice == 8) {
            undoOperation();
        }
        else if (choice == 9) {
            redoOperation();
        }
        else if (choice == 10) {
            cout << "\n=== SAVE TO FILE ===\n";
            string filename = getStringInput("Enter filename to save (e.g., products.txt): ");
            saveToFile(filename);
        }
        else if (choice == 11) {
            cout << "\n=== LOAD FROM FILE ===\n";
            string filename = getStringInput("Enter filename to load (e.g., products.txt): ");
            
            cout << "Warning: Loading from file will replace current inventory. Continue? (y/n): ";
            char confirm;
            cin >> confirm;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            if (confirm == 'y' || confirm == 'Y') {
                loadFromFile(filename);
            } else {
                cout << "Load operation cancelled.\n";
            }
        }
        else if (choice == 12) {
            cout << "\nAre you sure you want to exit? Any unsaved changes will be lost. (y/n): ";
            char confirm;
            cin >> confirm;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            if (confirm == 'y' || confirm == 'Y') {
                cout << "Thank you for using Inventory Track Pro!\n";
                break;
            }
        }
    }
    
    return 0;
}