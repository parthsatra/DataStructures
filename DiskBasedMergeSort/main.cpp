#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <climits>
#include <cstring>
#include <string>
#include <sys/time.h>

using namespace std;

ifstream input_handle;
streampos input_size;
fstream run_file[1000];
ofstream output_handle;
int run_size[1000];
int total_runs = 0;
int total_super_runs = 0;
int heap[750];

void heapify(int heap[], int parent, int size) {
    // find left, right nodes
    int left = 2 * parent + 1;
    int right = 2 * parent + 2;

    // find the minimum of parent, left and right
    int min = parent;
    if (left < size && heap[left] < heap[min]) {
        min = left;
    }
    if (right < size && heap[right] < heap[min]) {
        min = right;
    }

    // swap the minimum with parent and re-heapify
    if (min != parent) {
        int temp = heap[parent];
        heap[parent] = heap[min];
        heap[min] = temp;
        heapify(heap, min, size);
    }
}

void build_heap(int heap[], int size) {
    for (int i = size / 2; i >= 0; i--) {
        heapify(heap, i, size);
    }
}

static int qsort_cmp(const void *a1, const void *a2) {
    return (*(int *) a1 - *(int *) a2);
}

void open_input(char *file_name) {
    input_handle.open(file_name, ios::in | ios::binary | ios::ate);
    if (input_handle.is_open()) {
        input_size = input_handle.tellg();
    } else {
        cout << "Input file cannot be read" << endl;
        exit(1);
    }
}

void close_input() {
    if (input_handle.is_open()) {
        input_handle.close();
    }
}

void write_run(int i, const char *file_name, int content[], int size) {
    ofstream run_file_handle;
    run_file_handle.open(file_name, ios::out | ios::binary);
    run_file_handle.seekp(0, ios::beg);
    if (run_file_handle.is_open()) {
        run_file_handle.write((char *) content, size * sizeof(int));
        run_file_handle.flush();
        run_file_handle.close();
    }
}

void load_runs(char *file_name) {
    for (int i = 0; i < total_runs; i++) {
        stringstream buffer_file;
        buffer_file << string(file_name) << "." << setfill('0') << setw(3) << i;
        run_file[i].open(buffer_file.str().c_str(), ios::in | ios::binary);
        run_file[i].seekg(0, ios::beg);
    }
}

void load_super_runs(char *file_name) {
    for (int i = 0; i < total_super_runs; i++) {
        stringstream buffer_file;
        buffer_file << string(file_name) << ".super." << setfill('0') << setw(3) << i;
        run_file[i].open(buffer_file.str().c_str(), ios::in | ios::binary);
        run_file[i].seekg(0, ios::beg);
    }
}

void close_runs() {
    for (int i = 0; i < total_runs; i++) {
        if (run_file[i].is_open()) {
            run_file[i].close();
        }
    }
}

void close_super_runs() {
    for (int i = 0; i < total_super_runs; i++) {
        if (run_file[i].is_open()) {
            run_file[i].close();
        }
    }
}

int merge(int start, int end, fstream runs[], int run_size[], ofstream &output_file_handle) {
    // Creating in memory buffer.
    int output_file_size = 0;
    int output_buffer[1000];
    int output_index = -1;
    int merge_size = end - start + 1;
    int max_buffer_size = 1000 / merge_size;
    int **merge_buffer = new int *[merge_size];
    for (int i = 0; i < merge_size; i++) {
        merge_buffer[i] = new int[max_buffer_size];

    }

    int max_read[1000];
    int buffer_index[1000];
    int buffer_size[1000];
    for (int i = 0; i < 1000; i++) {
        max_read[i] = 0;
        buffer_index[i] = 0;
        buffer_size[i] = 0;
    }

    // Writing to output file
    output_file_handle.seekp(0, ios::beg);
    int files_read = 0;

    // Load the initial buffer
    for (int i = 0; i < merge_size; i++) {
        if (max_read[i] < run_size[start + i]) {
            if (max_buffer_size <= run_size[start + i] - max_read[i]) {
                buffer_size[i] = max_buffer_size;
            } else if (run_size[start + i] > max_read[i]) {
                buffer_size[i] = run_size[start + i] - max_read[i];
            } else {
                buffer_size[i] = 0;
                files_read++;
            }
            runs[start + i].seekg(0, ios::beg);
            if (buffer_size[i] > 0) {
                run_file[start + i].read((char *) merge_buffer[i], buffer_size[i] * sizeof(int));
            }
            max_read[i] += buffer_size[i];
        }
    }

    // Merging the files
    while (files_read < merge_size) {
        // Finding the min value from the memory buffer
        int min_value = INT_MAX;
        int index = -1;
        for (int i = 0; i < merge_size; i++) {
            if (buffer_index[i] < buffer_size[i]) {
                if (merge_buffer[i][buffer_index[i]] < min_value) {
                    min_value = merge_buffer[i][buffer_index[i]];
                    index = i;
                }
            }
        }

        // Writing that element to output file
        output_index++;
        output_buffer[output_index] = min_value;
        output_file_size++;
        if (output_index == 999) {
            output_file_handle.write((char *) output_buffer, (output_index + 1) * sizeof(int));
            output_file_handle.flush();
            output_index = -1;
        }
        buffer_index[index]++;

        // update the in memory buffer
        if (buffer_index[index] == buffer_size[index]) {
            buffer_index[index] = 0;
            if (max_buffer_size <= run_size[start + index] - max_read[index]) {
                buffer_size[index] = max_buffer_size;
            } else if (run_size[start + index] > max_read[index]) {
                buffer_size[index] = run_size[start + index] - max_read[index];
            } else {
                buffer_size[index] = 0;
                files_read++;
            }
            if (buffer_size[index] > 0) {
                runs[start + index].read((char *) merge_buffer[index], buffer_size[index] * sizeof(int));
            }
            max_read[index] += buffer_size[index];
        }
    }

    if (output_index > -1) {
        output_file_handle.write((char *) output_buffer, (output_index + 1) * sizeof(int));
    }

    // Clear in memory buffer
    for (int i = 0; i < merge_size; i++) {
        delete[] merge_buffer[i];
    }
    delete[] merge_buffer;

    return output_file_size;
}

void create_run(char *input_file) {
    input_handle.seekg(0, ios::beg);
    int i = 0;
    int file_size = input_size / sizeof(int);
    int current_size = 0;
    while (file_size > 0) {
        int buffer[1000];
        if (file_size >= 1000) {
            current_size = 1000;
        } else {
            current_size = file_size;
        }
        input_handle.read((char *) buffer, current_size * sizeof(int));
        qsort(buffer, current_size, sizeof(int), qsort_cmp);
        stringstream buffer_file;
        buffer_file << string(input_file) << "." << setfill('0') << setw(3) << i;
        write_run(i, buffer_file.str().c_str(), buffer, current_size);
        run_size[i] = current_size;
        file_size = file_size - current_size;
        i++;
    }
    total_runs = i;
}

void create_replacement_run(char *input_file) {
    input_handle.seekg(0, ios::beg);
    int input_buf[250];
    int output_buf[1000];
    int file_size = input_size / sizeof(int);
    int input_index = 0;
    int input_size = 250;
    int output_size = 0;
    int output_index = -1;
    int heap_size = 750;
    int run_index = 0;
    int max_heap_size = 750;
    int cur_run_size = 0;git
    int sec_index = 0;
    bool is_staged = false;

    // initial loading of the heap
    if (file_size >= 1000) {
        input_handle.read((char *) heap, 750 * sizeof(int));
        input_handle.read((char *) input_buf, 250 * sizeof(int));
        file_size = file_size - 1000;
    } else if (file_size > 750) {
        input_handle.read((char *) heap, 750 * sizeof(int));
        input_handle.read((char *) input_buf, (file_size - 750) * sizeof(int));
        file_size = 0;
        input_size = file_size - 750;
    } else if (file_size > 0) {
        input_handle.read((char *) heap, file_size * sizeof(int));
        heap_size = file_size;
        max_heap_size = heap_size;
        input_size = 0;
        input_index = -1;
        file_size = 0;
    }

    // build the heap.
    build_heap(heap, heap_size);

    // Create first run
    stringstream buffer_file;
    buffer_file << string(input_file) << "." << setfill('0') << setw(3) << run_index;
    output_handle.open(buffer_file.str().c_str(), ios::out | ios::binary);
    output_handle.seekp(0, ios::beg);

    // Iterate over the heap to generate the runs
    while (file_size > 0 || input_size > 0 || heap_size > 0 || is_staged) {
        if (heap_size > 0) {
            output_index++;
            output_buf[output_index] = heap[0];

            // replace top element in heap
            if(input_index > -1) {
                if(input_buf[input_index] >= heap[0]) {
                    heap[0] = input_buf[input_index];
                    heapify(heap, 0, heap_size);
                    input_index ++;
                } else {
                    heap[0] = heap[heap_size - 1];
                    heap[heap_size - 1] = input_buf[input_index];
                    heap_size --;
                    is_staged = true;
                    if(heap_size > 0) {
                        heapify(heap, 0, heap_size);
                    }
                    input_index++;
                }
            } else {
                heap[0] = heap[heap_size - 1];
                heap_size--;
                if(heap_size > 0) {
                    heapify(heap, 0, heap_size);
                }
            }

            // Check if input buffer is empty
            if(input_index == input_size) {
                if(file_size >= 250) {
                    input_handle.read((char *) input_buf, 250 * sizeof(int));
                    file_size = file_size - 250;
                    input_size = 250;
                    input_index = 0;
                } else if(file_size > 0){
                    input_handle.read((char *) input_buf, file_size * sizeof(int));
                    file_size = 0;
                    input_size = file_size;
                    input_index = 0;
                } else {
                    input_index = -1;
                    input_size = 0;
                    sec_index = heap_size;
                }
            }

            // check if output buffer is full, write to run
            if(output_index == 999) {
                output_handle.write((char *)output_buf, 1000 * sizeof(int));
                cur_run_size += 1000;
                output_index = -1;
            }
        } else {
            if (output_index > -1) {
                output_handle.write((char *) output_buf, (output_index + 1) * sizeof(int));
            }
            cur_run_size += output_index + 1;
            run_size[run_index] = cur_run_size;
            output_handle.close();
            if(max_heap_size > heap_size && is_staged) {
                run_index++;
                cur_run_size = 0;
                stringstream next_file;
                next_file << string(input_file) << "." << setfill('0') << setw(3) << run_index;
                output_handle.open(next_file.str().c_str(), ios::out | ios::binary);
                output_handle.seekp(0, ios::beg);
                output_index = -1;
                if(input_index == -1) {
                    heap_size = max_heap_size - sec_index;
                    for(int i = 0; i < heap_size; i++) {
                        heap[i] = heap[sec_index + i];
                    }
                } else {
                    heap_size = max_heap_size;
                }
                build_heap(heap, heap_size);
                is_staged = false;
            }
        }
    }

    //handling the remaining heap
    if(heap_size == 0 && output_index > -1) {
        output_handle.write((char *) output_buf, (output_index + 1) * sizeof(int));
        cur_run_size += output_index + 1;
        run_size[run_index] = cur_run_size;
        output_handle.close();
    }

    total_runs = run_index + 1;
}

void basic_sort(char *input_file, char *output_file, bool is_replacement) {
    open_input(input_file);

    // create runs
    if (is_replacement) {
        create_replacement_run(input_file);
    } else {
        create_run(input_file);
    }

    close_input();
    load_runs(input_file);

    // create output file
    output_handle.open(output_file, ios::out | ios::binary);
    output_handle.seekp(0, ios::beg);

    // merge the basic runs
    int size = merge(0, total_runs - 1, run_file, run_size, output_handle);

    close_runs();

    if (output_handle.is_open()) {
        output_handle.close();
    }
}

void multistep_sort(char *input_file, char *output_file) {
    open_input(input_file);

    //create runs
    create_run(input_file);
    close_input();
    load_runs(input_file);

    total_super_runs = (total_runs / 15);
    if (total_runs % 15 > 0) {
        total_super_runs += 1;
    }

    // create super runs
    int super_run_size[1000];
    for (int i = 0; i < total_super_runs; i++) {
        stringstream buffer_file;
        buffer_file << string(input_file) << ".super." << setfill('0') << setw(3) << i;
        output_handle.open(buffer_file.str().c_str(), ios::out | ios::binary);
        output_handle.seekp(0, ios::beg);
        int start = i * 15;
        int end = start + 14;
        if (end > total_runs - 1) {
            end = total_runs - 1;
        }
        super_run_size[i] = merge(start, end, run_file, run_size, output_handle);
        output_handle.close();
    }
    close_runs();

    // load super runs
    load_super_runs(input_file);

    // create output file
    output_handle.open(output_file, ios::out | ios::binary);
    output_handle.seekp(0, ios::beg);

    // merge super runs
    int size = merge(0, total_super_runs - 1, run_file, super_run_size, output_handle);

    close_super_runs();
    if (output_handle.is_open()) {
        output_handle.close();
    }
}

int main(int argc, char *argv[]) {
    if (argc == 4) {
        char *merge_mode = argv[1];
        char *input_file = argv[2];
        char *sorted_file = argv[3];

        struct timeval start_tm;
        struct timeval end_tm;
        gettimeofday(&start_tm, NULL);

        if (!strcmp("--basic", merge_mode)) {
            basic_sort(input_file, sorted_file, false);
        } else if (!strcmp("--multistep", merge_mode)) {
            multistep_sort(input_file, sorted_file);
        } else if (!strcmp("--replacement", merge_mode)) {
            basic_sort(input_file, sorted_file, true);
        } else {
            cout << "Invalid merge method specified";
            exit(1);
        }

        // Calculating the total time.
        gettimeofday(&end_tm, NULL);
        long sec = end_tm.tv_sec - start_tm.tv_sec;
        long usec = end_tm.tv_usec - start_tm.tv_usec;
        if (usec < 0) {
            usec = 1000000 + usec;
            sec = sec - 1;
        }
        printf("Time: %ld.%06ld", sec, usec);
        printf("\n");

    } else {
        cout << "Invalid parameter list passed" << endl;
        exit(1);
    }
    return 0;
}