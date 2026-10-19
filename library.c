#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_LEN   50
#define PERSON_LEN 40
#define DATA_FILE  "books.txt"

typedef struct WaitNode {
    char student[PERSON_LEN];
    struct WaitNode *next;
} WaitNode;

typedef struct Book {
    int id;
    char title[NAME_LEN];
    char author[NAME_LEN];
    char issuedTo[PERSON_LEN];
    WaitNode *wFront, *wRear;
    struct Book *next;
} Book;

typedef struct TreeNode {
    int id;
    Book *book;
    struct TreeNode *left, *right;
} TreeNode;

typedef struct Action {
    char type;
    int bookId;
    char s1[PERSON_LEN];
    char s2[PERSON_LEN];
    struct Action *next;
} Action;

Book     *head = NULL;
TreeNode *root = NULL;
Action   *top  = NULL;
int totalBooks = 0;

void copyStr(char *dst, const char *src, int size) {
    strncpy(dst, src, size - 1);
    dst[size - 1] = '\0';
}

void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
    for (int i = 0; buf[i]; i++)
        if (buf[i] == ';') buf[i] = ',';
}

int readInt(const char *prompt) {
    char line[50];
    int x;
    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof line, stdin) == NULL) return 0;
        if (sscanf(line, "%d", &x) == 1) return x;
        printf("Please enter a number.\n");
    }
}

void push(char type, int id, const char *s1, const char *s2) {
    Action *a = (Action *)malloc(sizeof(Action));
    a->type = type;
    a->bookId = id;
    copyStr(a->s1, s1, PERSON_LEN);
    copyStr(a->s2, s2, PERSON_LEN);
    a->next = top;
    top = a;
}

TreeNode *bstInsert(TreeNode *r, Book *b) {
    if (r == NULL) {
        TreeNode *n = (TreeNode *)malloc(sizeof(TreeNode));
        n->id = b->id;
        n->book = b;
        n->left = n->right = NULL;
        return n;
    }
    if (b->id < r->id) r->left = bstInsert(r->left, b);
    else if (b->id > r->id) r->right = bstInsert(r->right, b);
    return r;
}

Book *bstSearch(TreeNode *r, int id) {
    while (r != NULL) {
        if (id == r->id) return r->book;
        r = (id < r->id) ? r->left : r->right;
    }
    return NULL;
}

TreeNode *bstDelete(TreeNode *r, int id) {
    if (r == NULL) return NULL;
    if (id < r->id) r->left = bstDelete(r->left, id);
    else if (id > r->id) r->right = bstDelete(r->right, id);
    else {
        if (r->left == NULL) { TreeNode *t = r->right; free(r); return t; }
        if (r->right == NULL) { TreeNode *t = r->left; free(r); return t; }
        TreeNode *s = r->right;
        while (s->left != NULL) s = s->left;
        r->id = s->id;
        r->book = s->book;
        r->right = bstDelete(r->right, s->id);
    }
    return r;
}

void printBookRow(Book *b) {
    printf("%-6d %-30.30s %-22.22s %s\n", b->id, b->title, b->author,
           b->issuedTo[0] ? b->issuedTo : "Available");
}

void printHeader() {
    printf("%-6s %-30s %-22s %s\n", "ID", "Title", "Author", "Status (Issued To)");
    printf("---------------------------------------------------------------------------------\n");
}

void bstInorder(TreeNode *r) {
    if (r == NULL) return;
    bstInorder(r->left);
    printBookRow(r->book);
    bstInorder(r->right);
}

void freeTree(TreeNode *r) {
    if (r == NULL) return;
    freeTree(r->left);
    freeTree(r->right);
    free(r);
}

void enqueue(Book *b, const char *name) {
    WaitNode *n = (WaitNode *)malloc(sizeof(WaitNode));
    copyStr(n->student, name, PERSON_LEN);
    n->next = NULL;
    if (b->wRear == NULL) b->wFront = b->wRear = n;
    else { b->wRear->next = n; b->wRear = n; }
}

int dequeue(Book *b, char *out) {
    if (b->wFront == NULL) return 0;
    WaitNode *t = b->wFront;
    copyStr(out, t->student, PERSON_LEN);
    b->wFront = t->next;
    if (b->wFront == NULL) b->wRear = NULL;
    free(t);
    return 1;
}

void enqueueFront(Book *b, const char *name) {
    WaitNode *n = (WaitNode *)malloc(sizeof(WaitNode));
    copyStr(n->student, name, PERSON_LEN);
    n->next = b->wFront;
    b->wFront = n;
    if (b->wRear == NULL) b->wRear = n;
}

void removeLastWaiting(Book *b) {
    if (b->wFront == NULL) return;
    if (b->wFront == b->wRear) {
        free(b->wFront);
        b->wFront = b->wRear = NULL;
        return;
    }
    WaitNode *t = b->wFront;
    while (t->next != b->wRear) t = t->next;
    free(b->wRear);
    t->next = NULL;
    b->wRear = t;
}

int waitingCount(Book *b) {
    int c = 0;
    for (WaitNode *t = b->wFront; t != NULL; t = t->next) c++;
    return c;
}

Book *addBookNode(int id, const char *title, const char *author, const char *issuedTo) {
    Book *b = (Book *)malloc(sizeof(Book));
    b->id = id;
    copyStr(b->title, title, NAME_LEN);
    copyStr(b->author, author, NAME_LEN);
    copyStr(b->issuedTo, issuedTo, PERSON_LEN);
    b->wFront = b->wRear = NULL;
    b->next = NULL;

    if (head == NULL) head = b;
    else {
        Book *t = head;
        while (t->next != NULL) t = t->next;
        t->next = b;
    }
    root = bstInsert(root, b);
    totalBooks++;
    return b;
}

void removeBook(int id) {
    Book *t = head, *prev = NULL;
    while (t != NULL && t->id != id) { prev = t; t = t->next; }
    if (t == NULL) return;

    if (prev) prev->next = t->next;
    else head = t->next;
    root = bstDelete(root, id);

    char tmp[PERSON_LEN];
    while (dequeue(t, tmp)) { }
    free(t);
    totalBooks--;
}

void addBook() {
    char title[NAME_LEN], author[NAME_LEN];
    int id = readInt("Enter Book ID (positive number): ");
    if (id <= 0) { printf("Invalid ID.\n"); return; }
    if (bstSearch(root, id) != NULL) { printf("A book with this ID already exists.\n"); return; }
    readLine("Enter Title : ", title, NAME_LEN);
    readLine("Enter Author: ", author, NAME_LEN);
    if (title[0] == '\0') { printf("Title cannot be empty.\n"); return; }
    addBookNode(id, title, author, "");
    push('A', id, title, "");
    printf("Book added successfully.\n");
}

void displayAll() {
    if (head == NULL) { printf("No books in the library.\n"); return; }
    printf("\nAll books (linked list order, total = %d)\n", totalBooks);
    printHeader();
    for (Book *t = head; t != NULL; t = t->next) printBookRow(t);
}

void displaySorted() {
    if (root == NULL) { printf("No books in the library.\n"); return; }
    printf("\nBooks sorted by ID (BST inorder traversal)\n");
    printHeader();
    bstInorder(root);
}

void searchBook() {
    int id = readInt("Enter Book ID to search: ");
    Book *b = bstSearch(root, id);
    if (b == NULL) { printf("Book not found.\n"); return; }
    printf("\nBook found!\n");
    printf("ID      : %d\nTitle   : %s\nAuthor  : %s\n", b->id, b->title, b->author);
    printf("Status  : %s\n", b->issuedTo[0] ? "Issued" : "Available");
    if (b->issuedTo[0]) {
        printf("Issued to: %s\n", b->issuedTo);
        printf("Students waiting: %d\n", waitingCount(b));
    }
}

void issueBook() {
    char student[PERSON_LEN];
    int id = readInt("Enter Book ID to issue: ");
    Book *b = bstSearch(root, id);
    if (b == NULL) { printf("Book not found.\n"); return; }
    readLine("Enter Student Name: ", student, PERSON_LEN);
    if (student[0] == '\0') { printf("Name cannot be empty.\n"); return; }

    if (b->issuedTo[0] == '\0') {
        copyStr(b->issuedTo, student, PERSON_LEN);
        push('I', id, student, "");
        printf("Book \"%s\" issued to %s.\n", b->title, student);
    } else if (strcmp(b->issuedTo, student) == 0) {
        printf("This book is already issued to %s.\n", student);
    } else {
        enqueue(b, student);
        push('W', id, student, "");
        printf("Book is already issued to %s.\n", b->issuedTo);
        printf("%s added to the waiting list (position %d).\n", student, waitingCount(b));
    }
}

void returnBook() {
    char returnedBy[PERSON_LEN], nextStudent[PERSON_LEN] = "";
    int id = readInt("Enter Book ID to return: ");
    Book *b = bstSearch(root, id);
    if (b == NULL) { printf("Book not found.\n"); return; }
    if (b->issuedTo[0] == '\0') { printf("This book is not issued.\n"); return; }

    copyStr(returnedBy, b->issuedTo, PERSON_LEN);
    if (dequeue(b, nextStudent)) {
        copyStr(b->issuedTo, nextStudent, PERSON_LEN);
        printf("Book returned by %s.\n", returnedBy);
        printf("Automatically issued to next student in queue: %s.\n", nextStudent);
    } else {
        b->issuedTo[0] = '\0';
        printf("Book returned by %s. It is now available.\n", returnedBy);
    }
    push('R', id, returnedBy, nextStudent);
}

void deleteBook() {
    int id = readInt("Enter Book ID to delete: ");
    Book *b = bstSearch(root, id);
    if (b == NULL) { printf("Book not found.\n"); return; }
    if (b->issuedTo[0]) { printf("Cannot delete: book is currently issued.\n"); return; }
    push('D', id, b->title, "");
    removeBook(id);
    printf("Book deleted.\n");
}

void viewWaiting() {
    int id = readInt("Enter Book ID: ");
    Book *b = bstSearch(root, id);
    if (b == NULL) { printf("Book not found.\n"); return; }
    if (b->wFront == NULL) { printf("No students waiting for this book.\n"); return; }
    printf("Waiting list for \"%s\" (front of queue first):\n", b->title);
    int pos = 1;
    for (WaitNode *t = b->wFront; t != NULL; t = t->next)
        printf("  %d. %s\n", pos++, t->student);
}

void describe(Action *a) {
    switch (a->type) {
        case 'A': printf("Added book %d (%s)\n", a->bookId, a->s1); break;
        case 'I': printf("Issued book %d to %s\n", a->bookId, a->s1); break;
        case 'W': printf("%s joined waiting list of book %d\n", a->s1, a->bookId); break;
        case 'R':
            printf("Book %d returned by %s", a->bookId, a->s1);
            if (a->s2[0]) printf(" (auto-issued to %s)", a->s2);
            printf("\n");
            break;
        case 'D': printf("Deleted book %d (%s)\n", a->bookId, a->s1); break;
    }
}

void showHistory() {
    if (top == NULL) { printf("No history yet.\n"); return; }
    printf("\nHistory (latest action first - STACK order):\n");
    int n = 1;
    for (Action *a = top; a != NULL; a = a->next) {
        printf("  %d. ", n++);
        describe(a);
    }
}

void undoLast() {
    if (top == NULL) { printf("Nothing to undo.\n"); return; }
    Action *a = top;
    top = a->next;
    Book *b = bstSearch(root, a->bookId);

    printf("Undoing: ");
    describe(a);

    switch (a->type) {
        case 'A':
            removeBook(a->bookId);
            break;
        case 'I':
            if (b) b->issuedTo[0] = '\0';
            break;
        case 'W':
            if (b) removeLastWaiting(b);
            break;
        case 'R':
            if (b) {
                if (a->s2[0]) enqueueFront(b, a->s2);
                copyStr(b->issuedTo, a->s1, PERSON_LEN);
            }
            break;
        case 'D':
            printf("Sorry, a deleted book cannot be restored.\n");
            free(a);
            return;
    }
    printf("Undo done.\n");
    free(a);
}

void saveBooks() {
    FILE *f = fopen(DATA_FILE, "w");
    if (f == NULL) { printf("Could not save to %s\n", DATA_FILE); return; }
    for (Book *t = head; t != NULL; t = t->next)
        fprintf(f, "%d;%s;%s;%s\n", t->id, t->title, t->author,
                t->issuedTo[0] ? t->issuedTo : "-");
    fclose(f);
    printf("Data saved to %s\n", DATA_FILE);
}

void loadBooks() {
    FILE *f = fopen(DATA_FILE, "r");
    if (f == NULL) return;
    char line[256], t[NAME_LEN], a[NAME_LEN], s[PERSON_LEN];
    int id;
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (sscanf(line, "%d;%49[^;];%49[^;];%39[^\n]", &id, t, a, s) == 4) {
            if (strcmp(s, "-") == 0) s[0] = '\0';
            if (id > 0 && bstSearch(root, id) == NULL) addBookNode(id, t, a, s);
        }
    }
    fclose(f);
}

void freeAll() {
    while (head != NULL) {
        Book *t = head;
        head = head->next;
        char tmp[PERSON_LEN];
        while (dequeue(t, tmp)) { }
        free(t);
    }
    freeTree(root);
    root = NULL;
    while (top != NULL) {
        Action *a = top;
        top = top->next;
        free(a);
    }
}

void showMenu() {
    printf("\n======= LIBRARY MANAGEMENT SYSTEM =======\n");
    printf(" 1. Add Book\n");
    printf(" 2. Display All Books            (Linked List)\n");
    printf(" 3. Display Books Sorted by ID   (BST Inorder)\n");
    printf(" 4. Search Book by ID            (BST Search)\n");
    printf(" 5. Issue Book\n");
    printf(" 6. Return Book\n");
    printf(" 7. Delete Book\n");
    printf(" 8. View Waiting List            (Queue)\n");
    printf(" 9. View History                 (Stack)\n");
    printf("10. Undo Last Action             (Stack Pop)\n");
    printf(" 0. Save and Exit\n");
    printf("=========================================\n");
}

int main() {
    loadBooks();
    printf("Welcome! %d book(s) loaded.\n", totalBooks);

    int choice;
    do {
        showMenu();
        choice = readInt("Enter your choice: ");
        switch (choice) {
            case 1:  addBook();      break;
            case 2:  displayAll();   break;
            case 3:  displaySorted();break;
            case 4:  searchBook();   break;
            case 5:  issueBook();    break;
            case 6:  returnBook();   break;
            case 7:  deleteBook();   break;
            case 8:  viewWaiting();  break;
            case 9:  showHistory();  break;
            case 10: undoLast();     break;
            case 0:  break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 0);

    saveBooks();
    freeAll();
    printf("Thank you for using the Library Management System.\n");
    return 0;
}
