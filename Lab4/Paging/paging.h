//
// Created by Chrome on 4/17/17.
//

#ifndef PAGING_PAGING_H
#define PAGING_PAGING_H

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <string>
#include <sstream> // read file as standard input
#include <vector>
#include <deque>
#include <queue> // std::priority_queue
#include <algorithm>
#include <limits>
#include <iostream>

class Frame {
    int process_index, page_index;
public:
    Frame(int process_i = -1, int page_i = -1) {
        process_index = process_i;
        page_index = page_i;
    }
    int get_PrI() const { return process_index; }
    int get_PI() const { return page_index; }
    bool find_Frame(int process_i, int page_i) {
        return (process_i == process_index && page_i == page_index);
    }
    bool is_free() {
        return (process_index == -1 && page_index == -1);
    }
    void change_Frame(int process_i, int page_i) {
        process_index = process_i;
        page_index = page_i;
    }
};

class Process {
    double A, B, C;
    int S, number_of_references, reference_word;
    bool complete;
public:
    Process(double a, double b, double c, int s, int n, int r, bool complete = false) {
        A = a;
        B = b;
        C = c;
        S = s;
        number_of_references = n;
        reference_word = r;
    }
    double get_A() const { return A; }
    double get_B() const { return B; }
    double get_C() const { return C; }
    int get_S() const { return S; }
    int get_N() const { return number_of_references; }
    int get_R() const { return reference_word; }
    void minus_one_N() { number_of_references -= 1; }
    void change_R(int new_r) { reference_word = new_r; }
    void set_complete() { complete = true; }
    bool is_complete() { return complete; }
};

int randomNextReference(double A, double B, double C, int S, int w, FILE *pFile) {
    double y;
    unsigned int r;
    if ( ! feof (pFile) )
        if ( fscanf (pFile , "%i" , &r) != EOF ) {
            std::cout << "random number: " << r << std::endl;
            y = r / (INT_MAX + 1.0);
            if (y < A) {
                return (w + 1) % S;
            }
            else if (y < (A + B)) {
                return (w - 5 + S) % S;
            }
            else if (y < (A + B + C)) {
                return (w + 4) % S;
            }
            else { // a random value in 0..S-1 each with probability (1-A-B-C)/S
            }
        }
    return 0;
}

int randomEvictingFrame(FILE *pFile, int frame_number) {
    unsigned int r;
    if ( ! feof (pFile) )
        if ( fscanf (pFile , "%i" , &r) != EOF ) {
            std::cout << "random number: " << r << std::endl;
            return (r % frame_number);
        }
    return 0;
}

bool all_processes_are_completed(std::vector<Process> processes) {
    for (int i = 0; i < processes.size(); i++) {
        if (!processes[i].is_complete()) {
            return false;
        }
    }
    return true;
}

void Paging(FILE *pFile, std::vector<Frame> frame_table, int frame_number, std::vector<Process> processes, int P, std::string R) {
    int q = 3;
    int time_count = 0;
    while (!all_processes_are_completed(processes)) {
        for (int i = 0; i < processes.size(); i++) {
            for (int ref = 0; ref < q; ref++) {
                time_count++;
                // Part 1:
                //   simulate this reference for this process
                bool hit = false;
                int process_i = i + 1;
                int page_i = processes[i].get_R() / P;
                // find the page in the frame table
                for (int j = 0; j < frame_table.size(); j++) {
                    if (frame_table[j].find_Frame(process_i, page_i)) {
                        hit = true;
                        std::cout << process_i << " references word " << processes[i].get_R()
                                  << " (page " << page_i << ") at time " << time_count << ": "
                                  << "Hit in frame " << j << std::endl;
                        break;
                    }
                }
                // if not found
                if (!hit) {
                    // if free frame exists
                    bool free = false;
                    for (int k = frame_number; k >= 0; k--) {
                        if (frame_table[k].is_free()) {
                            frame_table[k].change_Frame(process_i, page_i);
                            free = true;
                            std::cout << process_i << " references word " << processes[i].get_R()
                                      << " (page " << page_i << ") at time " << time_count << ": "
                                      << "Fault, using free frame " << k << std::endl;
                            break;
                        }
                    }
                    // if free frame doesn't exist, then pick a victim
                    if (!free) {
                        if (R.compare("lifo") == 0) {
                            std::cout << process_i << " references word " << processes[i].get_R()
                                      << " (page " << page_i << ") at time " << time_count << ": "
                                      << "Fault, evicting page " << frame_table[0].get_PI()
                                      << " of " << frame_table[0].get_PrI() << " from frame 0" << std::endl;
                            frame_table[0].change_Frame(process_i, page_i);
                        }
                        else if (R.compare("random") == 0) {
                            int index = randomEvictingFrame(pFile, frame_number);
                            std::cout << process_i << " references word " << processes[i].get_R()
                                      << " (page " << page_i << ") at time " << time_count << ": "
                                      << "Fault, evicting page " << frame_table[index].get_PI()
                                      << " of " << frame_table[index].get_PrI() << " from frame " << index << std::endl;
                            frame_table[index].change_Frame(process_i, page_i);
                        }
                        else if (R.compare("lru") == 0) {
                            // do something
                        }
                    }
                }
                // Part 2:
                //   calculate the next reference for this process
                processes[i].change_R(randomNextReference(processes[i].get_A(),
                                                          processes[i].get_B(),
                                                          processes[i].get_C(),
                                                          processes[i].get_S(),
                                                          processes[i].get_R(),
                                                          pFile));
                // if the process is done
                processes[i].minus_one_N();
                if (processes[i].get_N() == 0) {
                    processes[i].set_complete();
                    break;
                }
            }
        }
    }

}

#endif //PAGING_PAGING_H
