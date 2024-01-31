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
#include <algorithm> 

using namespace std;

typedef pair<int, int> ii;

int n; 
std::queue<ii> task_queue;
std::mutex task_mutex;

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

void thread_merge(vector<int>& array, vector<std::mutex> &locks) {
    while (!task_queue.empty()) {

        std::unique_lock<std::mutex> lock(task_mutex);
        if (task_queue.empty()) {
            break; 
        }
        ii temp = task_queue.front();
        task_queue.pop(); 
        lock.unlock(); 

        int s = temp.first; 
        int e = temp.second; 

        vector<std::unique_lock<std::mutex>> lockArray;

        // aquire locks in range 
        for (int i = s; i <= e; i++) {
            lockArray.emplace_back(locks[i]);
        } 

        merge(array, s, e);
    }
}

int main() {
    // TODO: Seed your randomizer
    std::mt19937 rng(42); 

    // TODO: Get array size and thread count from user
    n = pow(2, 23);
    int thread_count = 128;
    std::uniform_int_distribution<int> dist(1, n);

    // TODO: Generate a random array of given size
    /*std::vector<int> array(n);
    for (int &num : array) {
        num = dist(rng);
        std::cout << num << ", ";
    }*/
    std::vector<int> array(n);
    for (int i = 0; i < n; ++i) {
        array[i] = i + 1;
    }
    random_shuffle(array.begin(), array.end(), [](int n) { return rand() % n; });

    // Print the shuffled array
    /* 
    cout << "Shuffled Array: ";
    for (int num : array) {
        cout << num << " ";
    }
    cout << endl;
    */
    
    //std::cout << "" << std::endl; 

    auto start = chrono::high_resolution_clock::now();


    // TODO: Call the generate_intervals method to generate the merge sequence
    vector<ii> intervals = generate_intervals(0, n - 1); 
    for (ii& interval : intervals) {
        task_queue.push(interval); 
    }

    // TODO: Call merge on each interval in sequence
    std::vector<std::mutex> mutex_array(n);

    // Once you get the single-threaded version to work, it's time to implement 
    // the concurrent version. Good luck :)
    std::vector<std::thread> threads;

    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(thread_merge, std::ref(array), std::ref(mutex_array));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    /*
    for (int& element : array) {
        std::cout << element << ", ";
    }
    std::cout << "" << std::endl;*/
    // Check if the array is sorted
    bool isSorted = is_sorted(array.begin(), array.end());
    cout << "Concurrent execution time: " << duration.count() << " seconds" << endl;
    cout << "Array is " << (isSorted ? "sorted" : "not sorted") << endl;

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
        if (r_ptr == (int)right.size() || (l_ptr != (int)left.size() && left[l_ptr] <= right[r_ptr])) {
            array[i] = left[l_ptr];
            l_ptr++;
        }
        else {
            array[i] = right[r_ptr];
            r_ptr++;
        }
    }
}