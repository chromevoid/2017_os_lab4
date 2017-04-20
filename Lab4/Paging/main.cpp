#include "paging.h"

int main(int argc, char const *argv[]) {
    if (argc == 8) {
        int M = atoi(argv[1]);  // the machine size in words
        int P = atoi(argv[2]);  // the page size in words
        int S = atoi(argv[3]);  // the size of a process
        int J = atoi(argv[4]);  // the ‘‘job mix’’, which determines A, B, and C
        int N = atoi(argv[5]);  // the number of references for each process
        std::string R = argv[6];  // LIFO, RANDOM, or LRU
        int debugging = atoi(argv[7]);
        bool show_detail = debugging != 0;
        // initiate frame table
        std::vector<Frame> frame_table;
        int frame_number = M / P;
        for (int i = 0; i < frame_number; i++) {
            Frame tmp_f;
            frame_table.push_back(tmp_f);
        }
        // initiate Process vector
        std::vector<Process> processes;
        if (J == 1) {
            Process tmp_p(1, 0, 0, S, N, 111 % S);
            processes.push_back(tmp_p);
        }
        else if (J == 2) {
            for (int i = 0; i < 4; i++) {
                Process tmp_p(1, 0, 0, S, N, 111 * (i + 1) % S);
                processes.push_back(tmp_p);
            }
        }
        else if (J == 3) {
            for (int i = 0; i < 4; i++) {
                Process tmp_p(1, 0, 0, S, N, 111 * (i + 1) % S);
                processes.push_back(tmp_p);
            }
        }
        else if (J == 4) {
            Process tmp_p1(0.75, 0.25, 0, S, N, 111 % S);
            Process tmp_p2(0.75, 0, 0.25, S, N, 222 % S);
            Process tmp_p3(0.75, 0.125, 0.125, S, N, 333 % S);
            Process tmp_p4(0.5, 0.125, 0.125, S, N, 444 % S);
            processes.push_back(tmp_p1);
            processes.push_back(tmp_p2);
            processes.push_back(tmp_p3);
            processes.push_back(tmp_p4);
        }
        // do the process
        FILE * pFile;
        pFile = fopen ("random-numbers.txt" , "r");
        if (pFile == NULL) {
            perror ("Error opening file");
            return 0;
        }
        Paging(pFile, frame_table, frame_number, processes, P, R, show_detail);
        fclose (pFile);
    } else {
        std::cout << "Wrong arguments." << std::endl;
    }
    return 0;
}