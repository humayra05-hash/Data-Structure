#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>


#define MAX_WORD_LEN 100
#define MAX_DEF_LEN 2000
#define DB_FILE "lexicon_db.txt"

// New definitions for Authentication
#define USER_DB_FILE "users_db.txt"
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50


// 1. HELPER: LOWERCASE CONVERSION


void toLowerCase(char* str) 
{
    for (int i = 0; str[i]; i++) 
    {
        str[i] = tolower((unsigned char)str[i]);
    }
}


// 2. STRUCTURE DEFINITIONS


// Singly Linked List (Master Storage)
typedef struct WordNode 
{
    char word[MAX_WORD_LEN];
    char definition[MAX_DEF_LEN];
    struct WordNode* next;
} WordNode;

// Standard BST Node (For Benchmark Comparison)
typedef struct BSTNode 
{
    WordNode* data;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

// AVL Tree Node (Internal structure for the Set)
typedef struct AVLNode 
{
    WordNode* data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;


// EXPLICIT SET DATA STRUCTURE (ADTs)

typedef struct WordSet 
{
    AVLNode* root;
    int size;
} WordSet;

// Stack Node (Undo operations)
typedef enum 
{ 
    ACTION_INSERT, ACTION_DELETE 
} ActionType;

typedef struct StackNode 
{
    ActionType action;
    char word[MAX_WORD_LEN];
    char definition[MAX_DEF_LEN];
    struct StackNode* next;
} StackNode;

// Queue Node & Queue (Recent Searches)
typedef struct QueueNode 
{
    char searchWord[MAX_WORD_LEN];
    struct QueueNode* next;
} QueueNode;

typedef struct Queue 
{
    QueueNode* front;
    QueueNode* rear;
    int count;
} Queue;



// 3. GLOBALS & FORWARD DECLARATIONS


WordNode* masterListHead = NULL;
BSTNode* bstRoot = NULL;
WordSet* lexiconSet = NULL;
StackNode* undoStackTop = NULL;
Queue* searchHistory = NULL;

#define MAX_HISTORY 10

// Forward declarations
int insertLexiconEntry(const char* word, const char* def, int isLoadingFromDB);
void deleteLexiconEntry(const char* word, int isUndo);
void appendToDatabase(const char* word, const char* def);
WordNode* searchSet(WordSet* set, const char* word, int* comparisons);



//adding the avl,queue

WordSet* createWordSet() 
{
    WordSet* set = (WordSet*)malloc(sizeof(WordSet));
    set->root = NULL;
    set->size = 0;
    return set;
}

int getHeight(AVLNode* n) 
{ 
    return n ? n->height : 0; 
}
int max(int a, int b) 
{ 
    return (a > b) ? a : b; 
}
int getBalance(AVLNode* n) 
{ 
    return n ? getHeight(n->left) - getHeight(n->right) : 0; 
}

AVLNode* createAVLNode(WordNode* data) 
{
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

AVLNode* rightRotate(AVLNode* y) 
{
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    return x;
}

AVLNode* leftRotate(AVLNode* x) 
{
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    return y;
}

AVLNode* insertAVL(AVLNode* node, WordNode* data) 
{
    if (!node) 
        return createAVLNode(data);

    int cmp = strcmp(data->word, node->data->word);
    if (cmp < 0) 
        node->left = insertAVL(node->left, data);
    else if (cmp > 0) 
        node->right = insertAVL(node->right, data);
    else 
        return node; // Duplicate found, set behavior prevents duplicates

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    int balance = getBalance(node);

    if (balance > 1 && strcmp(data->word, node->left->data->word) < 0) 
        return rightRotate(node);
    if (balance < -1 && strcmp(data->word, node->right->data->word) > 0) 
        return leftRotate(node);
    if (balance > 1 && strcmp(data->word, node->left->data->word) > 0) 
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && strcmp(data->word, node->right->data->word) < 0) 
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

AVLNode* minValueAVLNode(AVLNode* node) 
{
    AVLNode* current = node;
    while (current && current->left != NULL) 
        current = current->left;
    return current;
}

AVLNode* deleteAVL(AVLNode* root, const char* word) 
{
    if (root == NULL) 
        return root;
    int cmp = strcmp(word, root->data->word);
    
    if (cmp < 0) 
        root->left = deleteAVL(root->left, word);
    else if (cmp > 0) 
        root->right = deleteAVL(root->right, word);
    else 
    {
        if ((root->left == NULL) || (root->right == NULL)) 
        {
            AVLNode* temp = root->left ? root->left : root->right;
            if (temp == NULL) 
            {
                temp = root; 
                root = NULL; 
            } 
            else 
                *root = *temp;
            free(temp);
        }
        else 
        {
            AVLNode* temp = minValueAVLNode(root->right);
            root->data = temp->data;
            root->right = deleteAVL(root->right, temp->data->word);
        }
    }

    if (root == NULL) 
        return root;
    root->height = 1 + max(getHeight(root->left), getHeight(root->right));
    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0) 
        return rightRotate(root);
    if (balance > 1 && getBalance(root->left) < 0) 
    {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0) 
        return leftRotate(root);
    if (balance < -1 && getBalance(root->right) > 0) 
    {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    return root;
}

WordNode* searchSet(WordSet* set, const char* word, int* comparisons) 
{
    AVLNode* current = set->root;
    while (current != NULL) 
    {
        if (comparisons) 
            (*comparisons)++;
        int cmp = strcmp(word, current->data->word);
        if (cmp == 0) 
            return current->data;
        if (cmp < 0) 
            current = current->left;
        else 
            current = current->right;
    }
    return NULL;
}


// =========================================================
// 5. QUEUE (SEARCH HISTORY) & OTHER HELPERS
// =========================================================

Queue* createQueue() 
{
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
    return q;
}

void dequeueHistory() 
{
    if (searchHistory->front == NULL) 
        return;
    QueueNode* temp = searchHistory->front;
    searchHistory->front = searchHistory->front->next;
    if (searchHistory->front == NULL) 
        searchHistory->rear = NULL;
    free(temp);
    searchHistory->count--;
}

void enqueueHistory(const char* word) 
{
    if (searchHistory == NULL) 
        searchHistory = createQueue();
    if (searchHistory->count == MAX_HISTORY) 
        dequeueHistory();

    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    strcpy(newNode->searchWord, word);
    newNode->next = NULL;

    if (searchHistory->rear == NULL) 
    {
        searchHistory->front = searchHistory->rear = newNode;
    } 
    else 
    {
        searchHistory->rear->next = newNode;
        searchHistory->rear = newNode;
    }
    searchHistory->count++;
}

void displaySearchHistory() 
{
    if (searchHistory == NULL || searchHistory->front == NULL) 
    {
        printf("\n[Search History] No recent searches.\n");
        return;
    }
    printf("\n--- Recent Searches (Last %d) ---\n", searchHistory->count);
    QueueNode* current = searchHistory->front;
    int index = 1;
    while (current != NULL) 
    {
        printf("%d. %s\n", index++, current->searchWord);
        current = current->next;
    }
    printf("----------------------------------\n");
}

WordNode* createWordNode(const char* word, const char* def) 
{
    WordNode* newNode = (WordNode*)malloc(sizeof(WordNode));
    strcpy(newNode->word, word);
    strcpy(newNode->definition, def);
    newNode->next = NULL;
    return newNode;
}

WordNode* insertIntoMasterList(const char* word, const char* def) 
{
    WordNode* newNode = createWordNode(word, def);
    newNode->next = masterListHead;
    masterListHead = newNode;
    return newNode;
}

BSTNode* createBSTNode(WordNode* data) 
{
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    return node;
}

BSTNode* insertBST(BSTNode* node, WordNode* data) 
{
    if (!node) 
        return createBSTNode(data);
    int cmp = strcmp(data->word, node->data->word);
    if (cmp < 0) 
        node->left = insertBST(node->left, data);
    else if (cmp > 0) 
        node->right = insertBST(node->right, data);
    return node;
}

BSTNode* minValueBSTNode(BSTNode* node) 
{
    BSTNode* current = node;
    while (current && current->left != NULL) 
        current = current->left;
    return current;
}

BSTNode* deleteBST(BSTNode* root, const char* word) 
{
    if (root == NULL) 
        return root;
    int cmp = strcmp(word, root->data->word);
    if (cmp < 0) 
        root->left = deleteBST(root->left, word);
    else if (cmp > 0) 
        root->right = deleteBST(root->right, word);
    else 
    {
        if (root->left == NULL) 
        {
            BSTNode* temp = root->right;
            free(root);
            return temp;
        } 
        else if (root->right == NULL) 
        {
            BSTNode* temp = root->left;
            free(root);
            return temp;
        }
        BSTNode* temp = minValueBSTNode(root->right);
        root->data = temp->data;
        root->right = deleteBST(root->right, temp->data->word);
    }
    return root;
}

WordNode* searchBST(BSTNode* root, const char* word, int* comparisons) 
{
    BSTNode* current = root;
    while (current != NULL) 
    {
        if (comparisons) 
            (*comparisons)++;
        int cmp = strcmp(word, current->data->word);
        if (cmp == 0) 
            return current->data;
        if (cmp < 0) 
            current = current->left;
        else 
            current = current->right;
    }
    return NULL;
}



// 6. FILE I/O


void loadDatabase() 
{
    FILE* file = fopen(DB_FILE, "r");
    if (!file) 
    {
        printf("No existing lexicon database file found. A new one will be created upon insertion.\n");
        return;
    }
    char line[MAX_WORD_LEN + MAX_DEF_LEN + 5];
    int count = 0;
    while (fgets(line, sizeof(line), file)) 
    {
        line[strcspn(line, "\r\n")] = 0;
        char* word = strtok(line, "|");
        char* def = strtok(NULL, "|");
        if (word && def) 
        {
            toLowerCase(word);
            if (insertLexiconEntry(word, def, 1)) 
                count++;
        }
    }
    fclose(file);
    printf("Successfully loaded %d entries into memory.\n", count);
}

void appendToDatabase(const char* word, const char* def) 
{
    FILE* file = fopen(DB_FILE, "a");
    if (file) 
    {
        fprintf(file, "%s|%s\n", word, def);
        fclose(file);
    }
}

void syncDatabaseToFile() 
{
    FILE* file = fopen(DB_FILE, "w");
    if (!file) return;
    WordNode* current = masterListHead;
    while (current != NULL) 
    {
        if (searchSet(lexiconSet, current->word, NULL) != NULL) 
        {
            fprintf(file, "%s|%s\n", current->word, current->definition);
        }
        current = current->next;
    }
    fclose(file);
}



// 7.LEXICON CORE OPERATIONS & stack (UNDO) LOgic


void pushUndoAction(ActionType action, const char* word, const char* def) 
{
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    newNode->action = action;
    strcpy(newNode->word, word);
    strcpy(newNode->definition, def);
    newNode->next = undoStackTop;
    undoStackTop = newNode;
}

void processUndo() 
{
    if (undoStackTop == NULL) 
    {
        printf("\n[Undo] Nothing to undo!\n");
        return;
    }
    StackNode* lastAction = undoStackTop;
    undoStackTop = undoStackTop->next;

    if (lastAction->action == ACTION_INSERT) 
    {
        printf("\n[Undo] Undoing Insertion. Deleting '%s'...\n", lastAction->word);
        deleteLexiconEntry(lastAction->word, 1);
    } 
    else if (lastAction->action == ACTION_DELETE) 
    {
        printf("\n[Undo] Undoing Deletion. Restoring '%s'...\n", lastAction->word);
        insertLexiconEntry(lastAction->word, lastAction->definition, 1);
        appendToDatabase(lastAction->word, lastAction->definition);
    }
    free(lastAction);
}

int insertLexiconEntry(const char* word, const char* def, int isLoadingFromDB) 
{
    if (searchSet(lexiconSet, word, NULL) != NULL) 
    {
        if (!isLoadingFromDB) 
            printf("\n[Error] The word '%s' already exists in the set!\n", word);
        return 0;
    }
    WordNode* newEntry = insertIntoMasterList(word, def);
    bstRoot = insertBST(bstRoot, newEntry);
    lexiconSet->root = insertAVL(lexiconSet->root, newEntry);
    lexiconSet->size++;

    if (!isLoadingFromDB) 
    {
        appendToDatabase(word, def);
        pushUndoAction(ACTION_INSERT, word, def);
        printf("\n[Success] '%s' added to set and saved.\n", word);
    }
    return 1;
}
//isundo is 0 when user 
void deleteLexiconEntry(const char* word, int isUndo) 
{
    WordNode* target = searchSet(lexiconSet, word, NULL);
    if (target == NULL) 
    {
        if (!isUndo) printf("\n[Error] Word '%s' not found in set.\n", word);
        return;
    }
    if (!isUndo) 
        pushUndoAction(ACTION_DELETE, target->word, target->definition);

    bstRoot = deleteBST(bstRoot, word);
    lexiconSet->root = deleteAVL(lexiconSet->root, word);
    lexiconSet->size--;
    syncDatabaseToFile();

    if (!isUndo) 
        printf("\n[Success] '%s' deleted from set.\n", word);
}

void searchWordCommand(const char* target) 
{
    enqueueHistory(target);
    int setComps = 0, bstComps = 0;
    WordNode* result = searchSet(lexiconSet, target, &setComps);
    searchBST(bstRoot, target, &bstComps);

    if (result)
    {
        printf("\n[Found] %s: %s\n", result->word, result->definition);
        printf("[Benchmark] Set (AVL) Comparisons: %d | BST Comparisons: %d\n", setComps, bstComps);
    } 
    else 
    {
        printf("\n[Not Found] '%s' does not exist in the set.\n", target);
    }
}



// 8. AUTHENTICATION SYSTEM

int isUsernameTaken(const char* username) 
{
    FILE* file = fopen(USER_DB_FILE, "r");
    if (!file) return 0; // DB doesn't exist yet, username is free
    
    char line[MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 5];
    while (fgets(line, sizeof(line), file)) 
    {
        line[strcspn(line, "\r\n")] = 0; // Remove newline
        char* storedUser = strtok(line, "|");
        if (storedUser && strcmp(storedUser, username) == 0) 
        {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void registerUser() 
{
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    printf("\n--- Register ---\n");
    printf("Enter new username: ");
    fgets(username, MAX_USERNAME_LEN, stdin);
    username[strcspn(username, "\r\n")] = 0;

    if (isUsernameTaken(username)) 
    {
        printf("\n[Error] Username '%s' is already taken. Please try another.\n", username);
        return;
    }

    printf("Enter new password: ");
    fgets(password, MAX_PASSWORD_LEN, stdin);
    password[strcspn(password, "\r\n")] = 0;

    FILE* file = fopen(USER_DB_FILE, "a");
    if (file) 
    {
        fprintf(file, "%s|%s\n", username, password);
        fclose(file);
        printf("\n[Success] Account successfully created! You can now log in.\n");
    } 
    else 
    {
        printf("\n[Error] Could not access user database file.\n");
    }
}

int loginUser() 
{
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    printf("\n--- Log In ---\n");
    printf("Enter username: ");
    fgets(username, MAX_USERNAME_LEN, stdin);
    username[strcspn(username, "\r\n")] = 0;

    printf("Enter password: ");
    fgets(password, MAX_PASSWORD_LEN, stdin);
    password[strcspn(password, "\r\n")] = 0;

    FILE* file = fopen(USER_DB_FILE, "r");
    if (!file) 
    {
        printf("\n[Error] User database not found. Please register an account first.\n");
        return 0;
    }

    char line[MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 5];
    while (fgets(line, sizeof(line), file)) 
    {
        line[strcspn(line, "\r\n")] = 0;
        char* storedUser = strtok(line, "|");
        char* storedPass = strtok(NULL, "|");

        if (storedUser && storedPass && 
            strcmp(storedUser, username) == 0 && 
            strcmp(storedPass, password) == 0) 
        {
            fclose(file);
            printf("\n[Success] Logged in successfully. Welcome, %s!\n", username);
            return 1;
        }
    }
    fclose(file);
    printf("\n[Error] Invalid username or password.\n");
    return 0;
}


//##
// 9. MAIN MENUS & PROGRAM FLOW


void displayLexiconMenu() 
{
    printf("\n");
    printf("##       #######  ##   ##  #######  #######  #######  ##   ##\n");
    printf("##       ##        ## ##     ###    ##       ##   ##  ###  ##\n");
    printf("##       #####      ###      ###    ##       ##   ##  #### ##\n");
    printf("##       ##        ## ##     ###    ##       ##   ##  ## ####\n");
    printf("##       ##       ##   ##    ###    ##       ##   ##  ##  ###\n");
    printf("#######  #######  ##   ##  #######  #######  #######  ##   ##\n");

    printf("\n");
    printf("                   ####    ####    ####\n");
    printf("                  ##  ##  ##  ##  ##  ##\n");
    printf("                  ######  ######  ######\n");
    printf("                  ##  ##  ##      ##\n");
    printf("                  ##  ##  ##      ##\n");

    printf("\n");
    printf(" =========================================================\n");
    printf("1. Search for a Word\n");
    printf("2. Insert a New Word\n");
    printf("3. Delete a Word\n");
    printf("4. View Recent Search History (Queue)\n");
    printf("5. Undo Last Action (Stack)\n");
    printf("6. Log Out\n");
    printf("7. Exit Application\n");
    printf(" =========================================================\n");
    printf("Enter your choice: ");
}

int main() 
{
    int isLoggedIn = 0;
    int authChoice, appChoice;
    char word[MAX_WORD_LEN];
    char definition[MAX_DEF_LEN];

    // Initialization flags
    int lexiconInitialized = 0;

    while (1) 
    {
        if (!isLoggedIn) 
        {
            // --- Authentication Screen ---
            printf("\n =========================================================\n");
            printf("               WELCOME TO THE LEXICON APP\n");
            printf(" =========================================================\n");
            printf("1. Register\n");
            printf("2. Log In\n");
            printf("3. Exit\n");
            printf("Enter your choice: ");
            
            if (scanf("%d", &authChoice) != 1) 
            {
                while (getchar() != '\n'); 
                continue;
            }
            while (getchar() != '\n'); 

            switch (authChoice) 
            {
                case 1:
                    registerUser();
                    break;
                case 2:
                    isLoggedIn = loginUser();
                    // Load database into memory only once upon first successful login
                    if (isLoggedIn && !lexiconInitialized) 
                    {
                        lexiconSet = createWordSet();
                        searchHistory = createQueue();
                        printf("\nBooting up Lexicon Engine...\n");
                        loadDatabase();
                        lexiconInitialized = 1;
                    }
                    break;
                case 3:
                    printf("\nExiting program. Goodbye!\n");
                    return 0;
                default:
                    printf("\nInvalid choice. Please enter 1, 2, or 3.\n");
            }
        } 
        else 
        {
            // --- Lexicon App Screen ---
            displayLexiconMenu();
            if (scanf("%d", &appChoice) != 1) 
            {
                while (getchar() != '\n'); 
                continue;
            }
            while (getchar() != '\n'); 

            switch (appChoice) 
            {
                case 1:
                    printf("Enter word to search: ");
                    fgets(word, MAX_WORD_LEN, stdin);
                    word[strcspn(word, "\r\n")] = 0;
                    toLowerCase(word);
                    searchWordCommand(word);
                    break;
                case 2:
                    printf("Enter new word: ");
                    fgets(word, MAX_WORD_LEN, stdin);
                    word[strcspn(word, "\r\n")] = 0;
                    toLowerCase(word);
                    
                    printf("Enter definition: ");
                    fgets(definition, MAX_DEF_LEN, stdin);
                    definition[strcspn(definition, "\r\n")] = 0;
                    
                    insertLexiconEntry(word, definition, 0);
                    break;
                case 3:
                    printf("Enter word to delete: ");
                    fgets(word, MAX_WORD_LEN, stdin);
                    word[strcspn(word, "\r\n")] = 0;
                    toLowerCase(word);
                    
                    deleteLexiconEntry(word, 0);
                    break;
                case 4:
                    displaySearchHistory();
                    break;
                case 5:
                    processUndo();
                    break;
                case 6:
                    printf("\nLogging out... Returning to authentication menu.\n");
                    isLoggedIn = 0;
                    break;
                case 7:
                    printf("\nExiting program. Goodbye!\n");
                    return 0;
                default:
                    printf("\nInvalid choice. Enter 1-7.\n");
            }
        }
    }
    return 0;
}