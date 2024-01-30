#include <iostream>
#include <utility>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>
#include <ctime>
#include <cmath>
#include <queue>

using namespace std;

typedef pair<int, int> ii;

int n; 
std::queue<ii> task_queue;

/*
This function generates all the intervals for merge sort iteratively, given the
range of indices to sort. Algorithm runs in O(n).

Parameters:
start : int - start of range
end : int - end of range (inclusive)

Returns a list of integer pairs indicating the ranges for merge sort.
*/
vector<ii> generate_intervals(int start, int end);

/*
This function performs the merge operation of merge sort.

Parameters:
array : vector<int> - array to sort
s     : int         - start index of merge
e     : int         - end index (inclusive) of merge
*/
void merge(vector<int>& array, int s, int e);

void thread_merge(vector<int>& array, vector<std::mutex> &locks, int s, int e) {
    for (int i = s; i <= e; i++) {
        std::unique_lock<std::mutex> lock(locks[i]); 
    }

    merge(array, s, e); 
}

int main() {
    // TODO: Seed your randomizer
    std::mt19937 rng(std::time(nullptr)); 

    // TODO: Get array size and thread count from user
    n = 10; 
    int thread_count = 5; 
    std::uniform_int_distribution<int> dist(1, std::pow(2, 23));

    // TODO: Generate a random array of given size
    std::vector<int> array(n); 
    for (int &num : array) {
        num = dist(rng);
        std::cout << num << std::endl; 
    }

    // TODO: Call the generate_intervals method to generate the merge sequence
    vector<ii> intervals = generate_intervals(0, n - 1); 
    for (ii& interval : intervals) {
        task_queue.push(interval); 
    }

    // TODO: Call merge on each interval in sequence
    std::vector<std::mutex> mutexArray(n);

    // Once you get the single-threaded version to work, it's time to implement 
    // the concurrent version. Good luck :)

    
}

vector<ii> generate_intervals(int start, int end) {
    vector<ii> frontier;
    frontier.push_back(ii(start, end));
    int i = 0;
    while (i < (int)frontier.size()) {
        int s = frontier[i].first;
        int e = frontier[i].second;

        i++;

        // if base case
        if (s == e) {
            continue;
        }

        // compute midpoint
        int m = s + (e - s) / 2;

        // add prerequisite intervals
        frontier.push_back(ii(m + 1, e));
        frontier.push_back(ii(s, m));
    }

    vector<ii> retval;
    for (int i = (int)frontier.size() - 1; i >= 0; i--) {
        retval.push_back(frontier[i]);
    }
    return retval;
}

void merge(vector<int>& array, int s, int e) {
    // acquire lock s to e 
    int m = s + (e - s) / 2;
    vector<int> left;
    vector<int> right;
    // i = 1 and s = 1 and e = 1
    for (int i = s; i <= e; i++) {
        if (i <= m) {
            left.push_back(array[i]);
        }
        else {
            right.push_back(array[i]);
        }
    }
    int l_ptr = 0, r_ptr = 0;

    for (int i = s; i <= e; i++) {
        if (r_ptr == (int)right.size() || left[l_ptr] <= right[r_ptr]) {
            // aquire the lock[i] 
            array[i] = left[l_ptr];
            // release lock[i] 
            l_ptr++;
        }
        else {
            array[i] = right[r_ptr];
            r_ptr++;
        }
    }
}