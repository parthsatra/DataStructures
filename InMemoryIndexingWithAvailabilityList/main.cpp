#include <iostream>
#include <cstdlib>
#include <string.h>
#include <fstream>

using namespace std;

typedef struct {
    /* Record's key */
    int key;
    /* Record's offset in file */
    long off;
} index_S;

typedef struct {
    /* Hole's size */
    int siz;
    /* Hole's offset in file */
    long off;
} avail_S;

const int MAX_LENGTH = 10000;
index_S index_list[MAX_LENGTH];
avail_S avail_list[MAX_LENGTH];
int index_size = 0;
int avail_size = 0;
fstream db_handle;

void save_files() {
    fstream _a_handle;
    _a_handle.open("availability_list.db", ios::out | ios::binary | ios::trunc);
    if(_a_handle.is_open()) {
        _a_handle.write((char *)avail_list, (MAX_LENGTH * sizeof(avail_S))/ sizeof(char));
        _a_handle.close();
    }

    fstream _i_handle;
    _i_handle.open("index_list.db", ios::out | ios::binary | ios::trunc);
    if(_i_handle.is_open()) {
        _i_handle.write((char *)index_list, (MAX_LENGTH * sizeof(index_S))/ sizeof(char));
        _i_handle.close();
    }
}

void load_files() {
    fstream _a_handle;
    _a_handle.open("availability_list.db", ios::in | ios::binary);
    if(_a_handle.is_open()) {
        _a_handle.seekg(0);
        _a_handle.read((char *)avail_list, sizeof(avail_S));
        _a_handle.close();
    }

    fstream _i_handle;
    _i_handle.open("index_list.db", ios::in | ios::binary);
    if(_i_handle.is_open()) {
        _i_handle.seekg(0);
        _i_handle.read((char *) &index_list, sizeof(index_S));
        _i_handle.close();
    }

    for(int i = 0; i < MAX_LENGTH; i++) {
        if(avail_list[i].siz == -1) {
            avail_size = i;
            break;
        }
    }

    for(int i = 0; i < MAX_LENGTH; i++) {
        if(index_list[i].key == -1) {
            index_size = i;
            break;
        }
    }
}

void open_file(char *file_name) {
    db_handle.open(file_name, ios::in | ios::out | ios::binary);
    if (!db_handle.is_open()) {
        db_handle.open(file_name, ios::out | ios::binary);
        db_handle.close();
        db_handle.open(file_name, ios::in | ios::out | ios::binary);
    } else {
        load_files();
    }
}

void close_file() {
    if (db_handle.is_open()) {
        db_handle.close();
    }
}

void init() {
    for (int i = 0; i < MAX_LENGTH; i++) {
        index_list[i].key = -1;
        index_list[i].off = -1;
        avail_list[i].siz = -1;
        avail_list[i].off = -1;
    }
}

static int compare_index(const void *i1, const void *i2) {
    const index_S *_i1 = reinterpret_cast<const index_S *>(i1);
    const index_S *_i2 = reinterpret_cast<const index_S *>(i2);
    if (_i1->key < _i2->key) {
        return -1;
    } else if (_i1->key > _i2->key) {
        return 1;
    }
    return 0;
}

static int compare_best_fit(const void *a1, const void *a2) {
    const avail_S *_a1 = reinterpret_cast<const avail_S *>(a1);
    const avail_S *_a2 = reinterpret_cast<const avail_S *>(a2);
    if(_a1->siz < _a2->siz) {
        return -1;
    } else if(_a1->siz > _a2->siz){
        return 1;
    } else {
        if(_a1->off < _a2->off) {
            return -1;
        } else if(_a1->off > _a2->off) {
            return 1;
        }
        return 0;
    }
}

static int compare_worst_fit(const void *a1, const void *a2) {
    const avail_S *_a1 = reinterpret_cast<const avail_S *>(a1);
    const avail_S *_a2 = reinterpret_cast<const avail_S *>(a2);
    if(_a1->siz < _a2->siz) {
        return 1;
    } else if(_a1->siz > _a2->siz){
        return -1;
    } else {
        if(_a1->off < _a2->off) {
            return -1;
        } else if(_a1->off > _a2->off) {
            return 1;
        }
        return 0;
    }
}

int find(char *key) {
    int key_value = atoi(key);
    int low = 0;
    int high = index_size - 1;
    int i = -1;
    while (low <= high) {
        int mid = (high + low) / 2;
        if(index_list[mid].key == key_value) {
            i = mid;
            break;
        } else if(index_list[mid].key > key_value) {
            high = mid - 1;
        } else if(index_list[mid].key < key_value) {
            low = mid + 1;
        }
    }
    return i;
}

void shift(int pos, bool is_avail) {
    if(is_avail) {
        for(int i = pos + 1; i < avail_size; i++) {
            avail_list[i-1].siz = avail_list[i].siz;
            avail_list[i-1].off = avail_list[i].off;
        }
        avail_size --;
        avail_list[avail_size].siz = -1;
        avail_list[avail_size].off = -1;
    } else {
        for(int i = pos + 1; i < index_size; i++) {
            index_list[i-1].key = index_list[i].key;
            index_list[i-1].off = index_list[i].off;
        }
        index_size --;
        index_list[index_size].key = -1;
        index_list[index_size].key = -1;
    }
}

long get_offset(int record_size) {
    long offset = -1;
    for(int i = 0; i < avail_size; i++) {
        if(avail_list[i].siz >= (record_size + sizeof(int))) {
            offset = avail_list[i].off;
            int size = avail_list[i].siz - (record_size + sizeof(int));
            shift(i, true);
            if(size > 0) {
                avail_list[avail_size].siz = size;
                avail_list[avail_size].off = offset + record_size + sizeof(int);
            }
            break;
        }
    }
    return offset;
}

long index_to_offset(int index) {
    return index == -1 ? -1 : index_list[index].off;
}

void sort_avail(int ops) {
    if(ops == 2) {
        qsort(avail_list, avail_size, sizeof(avail_S), compare_best_fit);
    } else if(ops == 3) {
        qsort(avail_list, avail_size, sizeof(avail_S), compare_worst_fit);
    }
}

void add(char *key, char *record, int record_size, int ops) {
    // Check if key already present in index.
    int i = find(key);
    long offset = index_to_offset(i);
    if(offset != -1) {
        cout << "Record with SID=" << key << " exists" << endl;
        return;
    }
    // If not there then check the avail_list for empty space.
    offset = get_offset(record_size);

    // If offset is -1 then set offset to the end.
    if(offset == -1) {
        // Appending at the end
        if (db_handle.is_open()) {
            db_handle.seekg(0, ios::end);
            offset = db_handle.tellg();
        }
    } else {
        sort_avail(ops);
    }

    if (db_handle.is_open()) {
        db_handle.write((char *) &record_size, sizeof(int));
        db_handle.write(record, record_size);

        // Add to the index_list.
        index_list[index_size].key = atoi(key);
        index_list[index_size].off = offset;
        index_size++;

        // Sort the primary index
        qsort(index_list, index_size, sizeof(index_S), compare_index);
    }
}

void fetch_record(long offset, char* key) {
    if(offset == -1) {
        cout << "No record with SID=" << key << " exists" << endl;
    } else {
        if(db_handle.is_open()) {
            db_handle.seekg(offset);
            int record_size;
            db_handle.read((char*)&record_size, sizeof(int));
            char record[1000];
            db_handle.read(record, record_size);
            record[record_size] = '\0';
            cout << record << endl;
        }
    }
}

void del(char *key, int ops) {
    int i = find(key);
    long offset = index_to_offset(i);
    if(offset == -1) {
        cout << "No record with SID=" << key << " exists" << endl;
    } else {
        if(db_handle.is_open()) {
            db_handle.seekg(offset);
            int record_size;
            db_handle.read((char*)&record_size, sizeof(int));

            // add to availability list
            avail_list[avail_size].off = offset;
            avail_list[avail_size].siz = record_size + sizeof(int);
            avail_size ++;

            // sort the availability list
            sort_avail(ops);

            // remove from index file
            index_list[i].key = -1;
            index_list[i].off = -1;
            shift(i, false);
        }
    }

}

void print_result() {
    // Printing Index
    cout << "Index:" << endl;
    for(int i = 0; i < index_size; i++) {
        printf( "key=%d: offset=%ld\n", index_list[i].key, index_list[i].off );
    }

    // Printing Availability List
    cout << "Availability:" << endl;
    int hole_n = 0;
    int hole_siz = 0;
    for(int i = 0; i < avail_size; i++) {
        printf( "size=%d: offset=%ld\n", avail_list[i].siz, avail_list[i].off );
        hole_n ++;
        hole_siz += avail_list[i].siz;
    }
    //Printing total number and size of holes.
    printf( "Number of holes: %d\n", hole_n );
    printf( "Hole space: %d\n", hole_siz );
}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        int ops = 1;
        char *mode = argv[1];
        if(!strcmp("--best-fit", mode)) {
            ops = 2;
        } else if(!strcmp("--worst-fit", mode)) {
            ops = 3;
        }
        char *file_name = argv[2];
        // create/open db file.
        init();
        open_file(file_name);
        const int MAX_BUFFER = 2000;
        // take input
        while (true) {
            char input_buffer[MAX_BUFFER];
            cin.getline(input_buffer, MAX_BUFFER);
            int command_size = 1;
            int key_size = 1;
            int record_size = 1;
            int count = 0;

            // parse the command line input
            for (int i = 0; i < MAX_BUFFER; i++) {
                if (input_buffer[i] == ' ') {
                    if (count == 0) {
                        command_size = i;
                    } else if (count == 1) {
                        key_size = i - command_size - 1;
                    }
                    count++;
                }
                if (input_buffer[i] == '\0') {
                    if (count == 0) {
                        command_size = i;
                    } else if (count == 1) {
                        key_size = i - command_size - 1;
                    } else if (count == 2) {
                        record_size = i - key_size - command_size - 2;
                    }
                    break;
                }
            }

            // Perform the required operation.
            char command[1000];
            char key[1000];
            char record[1000];
            memcpy(command, input_buffer, command_size);
            command[command_size] = '\0';
            if (!strcmp("end", command)) {
                print_result();
                save_files();
                break;
            } else if (!strcmp("add", command)) {
                memcpy(key, input_buffer + command_size + 1, key_size);
                key[key_size] = '\0';
                memcpy(record, input_buffer + command_size + key_size + 2, record_size);
                record[record_size] = '\0';
                add(key, record, record_size, ops);
            } else if (!strcmp("del", command)) {
                memcpy(key, input_buffer + command_size + 1, key_size);
                del(key, ops);
            } else if (!strcmp("find", command)) {
                memcpy(key, input_buffer + command_size + 1, key_size);
                int i = find(key);
                long offset = index_to_offset(i);
                fetch_record(offset, key);
            }
        }
        close_file();
    }
    return 0;
}