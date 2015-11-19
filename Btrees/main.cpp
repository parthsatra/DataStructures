#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

class btree_node {      /* B-tree node */
public:
    int n;              /* Number of keys in node */
    int *key;           /* Node's keys */
    long *child;        /* Node's child subtree offsets */

    btree_node(int order) {
        n = 0;
        key = new int[order - 1];
        child = new long[order];
        for (int i = 0; i < order; i++) {
            child[i] = -1;
        }
    }
};

class add_node {        /* Node to maintain recursive call stack while insert */
public:
    bool is_added;      /* Checks if the key is already added */
    int key;            /* Key to be added */
    long right;         /* Right child offset */
};

class queue_node {      /* Node for maintaining a queue */
public:
    long offset;        /* Keeps the offset of the btree node in queue */
    queue_node *next;   /* Pointer to the next node in queue */
};

fstream index_handle;
long root;
int order;

/**
 * This method open/creates the index file if not present and extracts the root of the index.
 * Root is -1 in case the file is newly created.
 */
void open_index_file(char *file_name) {
    index_handle.open(file_name, ios::in | ios::out | ios::binary);
    if (!index_handle.is_open()) {
        index_handle.open(file_name, ios::out | ios::binary);
        index_handle.close();
        index_handle.open(file_name, ios::in | ios::out | ios::binary);
        root = -1;
    } else {
        index_handle.seekg(0, ios::beg);
        index_handle.read((char *) &root, sizeof(long));
    }
}

void write_node(btree_node node) {
    index_handle.write((char *) &node.n, sizeof(int));
    index_handle.write((char *) node.key, (order - 1) * sizeof(int));
    index_handle.write((char *) node.child, (order) * sizeof(long));
}

void read_node(btree_node *node) {
    index_handle.read((char *) &node->n, sizeof(int));
    index_handle.read((char *) node->key, (order - 1) * sizeof(int));
    index_handle.read((char *) node->child, (order) * sizeof(long));
}

/**
 * Recursive call to add the key in the tree.
 */
void add(long n_offset, add_node &new_node) {
    // read the current node
    btree_node node(order);
    index_handle.seekg(n_offset, ios::beg);
    read_node(&node);

    // Explore the right branch of the tree.
    bool is_explored = false;
    for (int i = 0; i < node.n; i++) {
        if (new_node.key == node.key[i]) {
            cout << "Entry with key=" << new_node.key << " already exists" << endl;
            new_node.is_added = true;
            return;
        } else if (new_node.key < node.key[i] && node.child[i] != -1) {
            is_explored = true;
            add(node.child[i], new_node);
            break;
        }
    }
    if (!is_explored && node.child[node.n] != -1) {
        add(node.child[node.n], new_node);
    }

    // If node already added, then return
    if (new_node.is_added) {
        return;
    }

    // Add the key to the node
    if (node.n < order - 1) {
        // Enough room to add current node. So no split needed.
        btree_node node1(order);
        node1.n = node.n + 1;
        node1.child[0] = node.child[0];
        bool is_inserted = false;
        for (int i = 0; i < node.n; i++) {
            if (new_node.key > node.key[i]) {
                node1.key[i] = node.key[i];
                node1.child[i + 1] = node.child[i + 1];
            } else {
                node1.key[i] = new_node.key;
                node1.child[i + 1] = new_node.right;
                while (i < node.n) {
                    node1.key[i + 1] = node.key[i];
                    node1.child[i + 2] = node.child[i + 1];
                    i++;
                }
                is_inserted = true;
                break;
            }
        }
        if (!is_inserted) {
            node1.key[node.n] = new_node.key;
            node1.child[node.n + 1] = new_node.right;
        }
        index_handle.seekp(n_offset, ios::beg);
        write_node(node1);
        new_node.is_added = true;
        return;
    } else {
        // Need to split the node
        int m = ((order - 1) / 2) + ((order - 1) % 2 != 0);
        int *key = new int[order];
        long *child = new long[order + 1];
        child[0] = node.child[0];
        bool is_inserted = false;
        for (int i = 0; i < order - 1; i++) {
            if (new_node.key > node.key[i]) {
                key[i] = node.key[i];
                child[i + 1] = node.child[i + 1];
            } else {
                key[i] = new_node.key;
                child[i + 1] = new_node.right;
                while (i < node.n) {
                    key[i + 1] = node.key[i];
                    child[i + 2] = node.child[i + 1];
                    i++;
                }
                is_inserted = true;
                break;
            }
        }
        if (!is_inserted) {
            key[order - 1] = new_node.key;
            child[order] = new_node.right;
        }

        btree_node left_node(order);
        btree_node right_node(order);

        // Writing the new left node
        left_node.n = m;
        left_node.child[0] = child[0];
        for (int i = 0; i < m; i++) {
            left_node.key[i] = key[i];
            left_node.child[i + 1] = child[i + 1];
        }
        index_handle.seekp(n_offset, ios::beg);
        write_node(left_node);

        // Writing the right node
        right_node.n = order - m - 1;
        right_node.child[0] = child[m + 1];
        int i = 0;
        for (int j = m + 1; j < order; j++) {
            right_node.key[i] = key[j];
            right_node.child[i + 1] = child[j + 1];
            i++;
        }
        index_handle.seekp(0, ios::end);
        long r_offset = index_handle.tellp();
        write_node(right_node);

        if (n_offset == root) {
            // Current node is root node
            btree_node n_root(order);
            n_root.n = 1;
            n_root.key[0] = key[m];
            n_root.child[0] = n_offset;
            n_root.child[1] = r_offset;
            index_handle.seekp(0, ios::end);
            root = index_handle.tellp();
            write_node(n_root);
            index_handle.seekp(0, ios::beg);
            index_handle.write((char *) &root, sizeof(long));
            new_node.is_added = true;
        } else {
            // Add the median to the parent.
            new_node.key = key[m];
            new_node.right = r_offset;
        }

        delete[] key;
        delete[] child;
    }
}

/**
 * Inserts the key to the index.
 */
void add(int key) {
    if (root == -1) {
        btree_node r_node(order);
        r_node.n = 1;
        r_node.key[0] = key;
        root = 0;
        index_handle.seekp(0, ios::beg);
        index_handle.write((char *)&root, sizeof(long));
        index_handle.seekp(0, ios::end);
        root = index_handle.tellp();
        write_node(r_node);
        index_handle.seekp(0, ios::beg);
        index_handle.write((char *)&root, sizeof(long));
    } else {
        add_node new_node;
        new_node.is_added = false;
        new_node.key = key;
        new_node.right = -1;
        add(root, new_node);
    }
}

void find(long offset, int key) {
    // read the current node
    btree_node node(order);
    index_handle.seekg(offset, ios::beg);
    read_node(&node);

    for(int i = 0; i < node.n; i++) {
        if(node.key[i] == key) {
            cout << "Entry with key=" << key << " exists" << endl;
            return;
        } else if(key < node.key[i]) {
            if(node.child[i] == -1) {
                cout << "Entry with key=" << key << " does not exist" << endl;
                return;
            } else {
                find(node.child[i], key);
                return;
            }
        }
    }
    if(node.child[node.n] == -1) {
        cout << "Entry with key=" << key << " does not exist" << endl;
    } else {
        find(node.child[node.n], key);
    }
}

void find(int key) {
    if(root == -1) {
        cout << "Entry with key=" << key << " does not exist" << endl;
    } else {
        find(root, key);
    }
}

void print() {
    queue_node *head = NULL;
    queue_node *tail = NULL;
    if(root == -1) {
        return;
    } else {
        head = new queue_node();
        head->offset = root;
        queue_node *delimit = new queue_node();
        delimit->offset = -1;
        delimit->next = NULL;
        head->next = delimit;
        tail = delimit;
    }

    bool is_leaf = false;
    int level = 1;
    cout << setw(2) << level << ": ";
    while(head != NULL) {
        if(head->offset == -1) {
            cout << endl;
            if(!is_leaf) {
                level++;
                cout << setw(2) << level << ": ";
                queue_node *delimit = new queue_node();
                delimit->offset = -1;
                delimit->next = NULL;
                tail->next = delimit;
                tail = delimit;
            }
        } else {
            btree_node node(order);
            index_handle.seekg(head->offset, ios::beg);
            read_node(&node);

            cout << node.key[0];
            for(int i = 1; i < node.n; i++) {
                cout << "," << node.key[i];
            }

            cout << " ";

            if(node.child[0] == -1) {
                is_leaf = true;
            }

            for(int i = 0; i <= node.n; i++) {
                if(node.child[i] == -1) {
                    break;
                } else {
                    queue_node *temp = new queue_node;
                    temp->offset = node.child[i];
                    temp->next = NULL;
                    tail->next = temp;
                    tail = temp;
                }
            }
        }

        queue_node *del = head;
        head = head->next;
        delete del;
    }
}

/**
 * Closes the index file.
 */
void close_index_file() {
    if (index_handle.is_open()) {
        index_handle.close();
    }
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        char *index_file_name = argv[1];
        order = atoi(argv[2]);

        // Open/Create index file.
        open_index_file(index_file_name);

        while (true) {
            string temp;
            getline(cin, temp);

            istringstream iss(temp);
            string command;
            string key;
            iss >> command;
            if (!strcmp("add", command.c_str())) {
                iss >> key;
                add(atoi(key.c_str()));
            } else if (!strcmp("find", command.c_str())) {
                iss >> key;
                find(atoi(key.c_str()));
            } else if (!strcmp("print", command.c_str())) {
                print();
            } else if (!strcmp("end", command.c_str())) {
                close_index_file();
                break;
            }
        }
    } else {
        cout << "Invalid number of parameters passed" << endl;
        exit(1);
    }
    return 0;
}