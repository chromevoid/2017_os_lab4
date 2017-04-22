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
    Frame(int process_i, int page_i) {
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

int randomNextReference(double A, double B, double C, int S, int w, FILE *pFile, bool show_detail) {
    double y;
    unsigned int r;
    if ( ! feof (pFile) ) {
        if (fscanf(pFile, "%i", &r) != EOF) {
            if (show_detail) { std::cout << "random number: " << r << std::endl; }
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
                if ( ! feof (pFile) ) {
                    if (fscanf(pFile, "%i", &r) != EOF) {
                        if (show_detail) { std::cout << "random number: " << r << std::endl; }
                        return (r % S);
                    }
                }
            }
        }
    }
    return 0;
}

int randomEvictingFrame(FILE *pFile, int frame_number, bool show_detail) {
    unsigned int r;
    if ( ! feof (pFile) ) {
        if (fscanf(pFile, "%i", &r) != EOF) {
            if (show_detail) { std::cout << "random number: " << r << std::endl; }
            return (r % frame_number);
        }
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

void all_lru_counts_add_one(std::vector<int> & lru_counts) {
    for (int i = 0; i < lru_counts.size(); i++) {
        lru_counts[i]++;
    }
}

void Paging(FILE *pFile, std::vector<Frame> frame_table, int frame_number, std::vector<Process> processes, int P, std::string R, bool show_detail) {
    int q = 3;
    int time_count = 0;
    std::vector<int> faults_recorder(processes.size(), 0);
    std::vector<int> temp;
    for (int i = 0; i < processes[0].get_S()/P; i++) {
        temp.push_back(0);
    }
    std::vector<std::vector<int>> evictions_recorder(processes.size(), temp);
    std::vector<std::vector<int>> loaded_time_recorder(processes.size(), temp);
    std::vector<std::vector<int>> residency_recorder(processes.size(), temp);
    bool lru = R.compare("lru") == 0;
    std::vector<int> lru_counts(frame_table.size(), 0);
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
                        if (lru) {
                            all_lru_counts_add_one(lru_counts);
                            lru_counts[j] = 0;
                        }
                        if (show_detail) {
                            std::cout << process_i << " references word " << processes[i].get_R()
                                      << " (page " << page_i << ") at time " << time_count << ": "
                                      << "Hit in frame " << j << std::endl;
                        }
                        break;
                    }
                }
                // if not found
                if (!hit) {
                    faults_recorder[i]++;
                    // if free frame exists
                    bool free = false;
                    for (int k = frame_number - 1; k >= 0; k--) {
                        if (frame_table[k].is_free()) {
                            frame_table[k].change_Frame(process_i, page_i);
                            free = true;
                            if (lru) {
                                all_lru_counts_add_one(lru_counts);
                                lru_counts[k] = 0;
                            }
                            // load
                            loaded_time_recorder[i][page_i] = time_count;
                            if (show_detail) {
                                std::cout << process_i << " references word " << processes[i].get_R()
                                          << " (page " << page_i << ") at time " << time_count << ": "
                                          << "Fault, using free frame " << k << std::endl;
                            }
                            break;
                        }
                    }
                    // if free frame doesn't exist, then pick a victim
                    if (!free) {
                        loaded_time_recorder[i][page_i] = time_count;
                        if (R.compare("lifo") == 0) {
                            if (show_detail) {
                                std::cout << process_i << " references word " << processes[i].get_R()
                                          << " (page " << page_i << ") at time " << time_count << ": "
                                          << "Fault, evicting page " << frame_table[0].get_PI()
                                          << " of " << frame_table[0].get_PrI() << " from frame 0" << std::endl;
                            }
                            evictions_recorder[frame_table[0].get_PrI() - 1][frame_table[0].get_PI()] += 1;
                            residency_recorder[frame_table[0].get_PrI() - 1][frame_table[0].get_PI()] +=
                                    time_count - loaded_time_recorder[frame_table[0].get_PrI() - 1][frame_table[0].get_PI()];
                            frame_table[0].change_Frame(process_i, page_i);
                        }
                        else if (R.compare("random") == 0) {
                            int index = randomEvictingFrame(pFile, frame_number, show_detail);
                            if (show_detail) {
                                std::cout << process_i << " references word " << processes[i].get_R()
                                          << " (page " << page_i << ") at time " << time_count << ": "
                                          << "Fault, evicting page " << frame_table[index].get_PI()
                                          << " of " << frame_table[index].get_PrI() << " from frame " << index << std::endl;
                            }
                            evictions_recorder[frame_table[index].get_PrI() - 1][frame_table[index].get_PI()] += 1;
                            residency_recorder[frame_table[index].get_PrI() - 1][frame_table[index].get_PI()] +=
                                    time_count - loaded_time_recorder[frame_table[index].get_PrI() - 1][frame_table[index].get_PI()];
                            frame_table[index].change_Frame(process_i, page_i);
                        }
                        else if (R.compare("lru") == 0) {
                            // do something
                            int lru_max = 0;
                            int lru_max_index = 0;
                            for (int k = 0; k < lru_counts.size(); k++) {
                                if (lru_max < lru_counts[k]) {
                                    lru_max = lru_counts[k];
                                    lru_max_index = k;
                                }
                            }
                            all_lru_counts_add_one(lru_counts);
                            lru_counts[lru_max_index] = 0;
                            if (show_detail) {
                                std::cout << process_i << " references word " << processes[i].get_R()
                                          << " (page " << page_i << ") at time " << time_count << ": "
                                          << "Fault, evicting page " << frame_table[lru_max_index].get_PI()
                                          << " of " << frame_table[lru_max_index].get_PrI() << " from frame " << lru_max_index << std::endl;
                            }
                            evictions_recorder[frame_table[lru_max_index].get_PrI() - 1][frame_table[lru_max_index].get_PI()] += 1;
                            residency_recorder[frame_table[lru_max_index].get_PrI() - 1][frame_table[lru_max_index].get_PI()] +=
                                    time_count - loaded_time_recorder[frame_table[lru_max_index].get_PrI() - 1][frame_table[lru_max_index].get_PI()];
                            frame_table[lru_max_index].change_Frame(process_i, page_i);
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
                                                          pFile, show_detail));
                // if the process is done
                processes[i].minus_one_N();
                if (processes[i].get_N() == 0) {
                    processes[i].set_complete();
                    break;
                }
            }
        }
    }
    if (show_detail) { std::cout << "\n"; }
    int total_faults = 0;
    int total_residency = 0;
    int total_evictions_number = 0;
    for (int index = 0; index < processes.size(); index++) {
        total_faults += faults_recorder[index];
        int total_residency_of_this_process = 0;
        int total_evictions_number_of_this_process = 0;
        for (int i = 0; i < residency_recorder[index].size(); i++) {
            total_residency_of_this_process += residency_recorder[index][i];
            total_evictions_number_of_this_process += evictions_recorder[index][i];
        }
        total_residency += total_residency_of_this_process;
        total_evictions_number += total_evictions_number_of_this_process;
        if (total_evictions_number_of_this_process == 0) {
            std::cout << "Process " << index + 1 << " had " << faults_recorder[index]
                      << " faults.\n\tWith no evictions, the average residence is undefined." << std::endl;
        }
        else {
            double result = total_residency_of_this_process * 1.0 / total_evictions_number_of_this_process;
            std::cout << "Process " << index + 1 << " had " << faults_recorder[index] << " faults and "
                      << result << " average residency" << std::endl;
        }
    }
    if (total_evictions_number == 0) {
        std::cout << "\nThe total number of faults is " << total_faults
                  << ".\n\tWith no evictions, the overall average residence is undefined." << std::endl;
    }
    else {
        double result = total_residency * 1.0 / total_evictions_number;
        std::cout << "\nThe total number of faults is " << total_faults
                  << " and the overall average residency is " << result << std::endl;
    }
}

#endif //PAGING_PAGING_H
