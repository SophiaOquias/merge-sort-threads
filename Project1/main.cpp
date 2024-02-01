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
#include <condition_variable>
#include <semaphore>

using namespace std;

typedef pair<int, int> ii;
int n; 
queue<ii> task_queue;
mutex task_mutex;
condition_variable task_condition;
condition_variable wait_condition; 
atomic<int> task_count(0);

bool flag = true; 

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

void thread_merge(vector<int>& array); 

int main() {
    // TODO: Seed your randomizer
    std::mt19937 rng(42); 

    // TODO: Get array size and thread count from user
    n = 1000; 
    int thread_count = 100; 
    std::uniform_int_distribution<int> dist(1, n);

    // TODO: Generate a random array of given size
    std::vector<int> array(n); 
    for (int &num : array) {
        num = dist(rng);
    }

    // TODO: Call the generate_intervals method to generate the merge sequence
    vector<ii> intervals = generate_intervals(0, n - 1); 

    // TODO: Call merge on each interval in sequence

    // Once you get the single-threaded version to work, it's time to implement 
    // the concurrent version. Good luck :)
    vector<thread> threads;

    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back(thread_merge, std::ref(array));
    }

    int max_depth = static_cast<int>(floor(log2(n))) + 1; // depth = floor(log_2(n)) + 1
    int arr_ptr = 0; 

    for (int k = max_depth + 1; k > 0; k--) {
        int num_leaves = static_cast<int>(pow(2, k - 1));

        if (k == max_depth + 1 && n != num_leaves) {
            num_leaves = 2 * n - num_leaves; 
        }
        
        int start = arr_ptr; 
        for (int i = arr_ptr; i < start + num_leaves; i++) {
            {
                unique_lock<mutex> lock(task_mutex);
                task_queue.push(intervals[arr_ptr]);
                task_count.fetch_sub(1);
                arr_ptr++;
            }

            task_condition.notify_one();
        }

        unique_lock<mutex> lock(task_mutex);
        wait_condition.wait(lock, [&]() { return task_count.load() == 0 || !flag; });

    }

    flag = false; 
    task_condition.notify_all();

    for (auto& thread : threads) {
        thread.join();
    }

    bool isSorted = is_sorted(array.begin(), array.end());
    cout << "Array is " << (isSorted ? "sorted" : "not sorted") << endl;

}

void thread_merge(vector<int>& array) {
    while (flag) {

        unique_lock<std::mutex> lock(task_mutex);

        task_condition.wait(lock, [&]() { return !flag || !task_queue.empty(); });

        if (!flag && task_queue.empty()) {
            lock.unlock();
            task_condition.notify_all();
            continue;  // Exit the loop if termination is requested and the queue is empty
        }

        ii temp = task_queue.front();
        task_queue.pop();
        lock.unlock();

        int s = temp.first;
        int e = temp.second;

        merge(array, s, e);

        task_count.fetch_add(1);
        wait_condition.notify_one();
    }
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