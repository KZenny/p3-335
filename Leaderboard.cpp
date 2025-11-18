#include "Leaderboard.hpp"

/**
 * @brief Constructor for RankingResult with top players, cutoffs, and elapsed time.
 *
 * @param top Vector of top-ranked Player objects, in sorted order.
 * @param cutoffs Map of player count thresholds to minimum level cutoffs.
 *   NOTE: This is only ever non-empty for Online::rankIncoming().
 *         This parameter & the corresponding member should be empty
 *         for all Offline algorithms.
 * @param elapsed Time taken to calculate the ranking, in seconds.
 */
RankingResult::RankingResult(const std::vector<Player>& top, const std::unordered_map<size_t, size_t>& cutoffs, double elapsed)
    : top_ { top }
    , cutoffs_ { cutoffs }
    , elapsed_ { elapsed }
{
}

namespace Offline {
/**
 * @brief Uses an early-stopping version of heapsort to
 *        select and sort the top 10% of players in-place
 *        (excluding the returned RankingResult vector)
 *
 * @param players A reference to the vector of Player objects to be ranked
 * @return A Ranking Result object whose
 * - top_ vector -> Contains the top 10% of players from the input in sorted order (ascending)
 * - cutoffs_    -> Is empty
 * - elapsed_    -> Contains the duration (ms) of the selection/sorting operation
 *
 * @post The order of the parameter vector is modified.
 */
RankingResult heapRank(std::vector<Player>& players) {
    //Start timer for elapsed_
    auto start = std::chrono::high_resolution_clock::now();
    
    //Create a max-heap from players vector
    std::make_heap(players.begin(), players.end());

    //Calculate 10% of the total players
    size_t topCount = players.size()/ 10;

    //Vector to store the top players
    std::vector<Player> topPlayers;

    //Extract the top players from players heap
    for (size_t i = 0; i < topCount; ++i) {
        std::pop_heap(players.begin(), players.end()); //Places the top player to the end of heap
        topPlayers.push_back(players.back()); //Add the top player to topPlayers
        players.pop_back(); // Removes the top player from the heap
    }

    //Sort the top players in ascending order using std::sort from <algorithm>
    std::sort(topPlayers.begin(), topPlayers.end());

    //Stop timer and calculate elapsed_
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    //Return a Ranking Result object 
    return RankingResult(topPlayers, {}, elapsed);
}

/**
 * @brief Uses a mixture of quickselect/quicksort to
 *        select and sort the top 10% of players with O(log N) memory
 *        (excluding the returned RankingResult vector)
 *
 * @param players A reference to the vector of Player objects to be ranked
 * @return A Ranking Result object whose
 * - top_ vector -> Contains the top 10% of players from the input in sorted order (ascending)
 * - cutoffs_    -> Is empty
 * - elapsed_    -> Contains the duration (ms) of the selection/sorting operation
 *
 * @post The order of the parameter vector is modified.
 */
RankingResult quickSelectRank(std::vector<Player>& players) {
    //Start timer for elapsed_
    auto start = std::chrono::high_resolution_clock::now();

    //Calculate the index to partition around for top 10%
    size_t N = players.size();
    size_t k = N - N / 10;

    //Use std::nth_element to partition the players vector
    std::nth_element(players.begin(), players.begin() + k, players.end());

    //Extract the top 10% players and sort them
    std::vector<Player> topPlayers(players.begin() + k, players.end());
    std::sort(topPlayers.begin(), topPlayers.end());

    //Stop timer and calculate elapsed_
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    //Return the Ranking Result object
    return RankingResult(topPlayers, {}, elapsed);
}
};
namespace Online {
/**
 * @brief A helper method that replaces the minimum element
 * in a min-heap with a target value & preserves the heap
 * by percolating the new value down to its correct position.
 *
 * Performs in O(log N) time.
 *
 * @pre The range [first, last) is a min-heap.
 *
 * @param first An iterator to a vector of Player objects
 *      denoting the beginning of a min-heap
 *      NOTE: Unlike the textbook, this is *not* an empty slot
 *      used to store temporary values. It is the root of the heap.
 *
 * @param last An iterator to a vector of Player objects
 *      denoting one past the end of a min-heap
 *      (i.e. it is not considering a valid index of the heap)
 *
 * @param target A reference to a Player object to be inserted into the heap
 * @post
 * - The vector slice denoted from [first,last) is a min-heap
 *   into which `target` has been inserted.
 * - The contents of `target` is not guaranteed to match its original state
 *   (ie. you may move it).
 */
void replaceMin(PlayerIt first, PlayerIt last, Player& target) {
    if (first == last) {
        return; // Empty heap, nothing to replace
    }

    // Replace the root of the heap with the target
    *first = target;

    // Percolate down to restore the min-heap property
    PlayerIt current = first;
    size_t heapSize = std::distance(first, last);

    while (true) {
        //Calculate indices of left and right children
        size_t leftChildIdx = 2 * std::distance(first, current) + 1;
        size_t rightChildIdx = 2 * std::distance(first, current) + 2;
        PlayerIt leftChild = first + leftChildIdx;
        PlayerIt rightChild = first + rightChildIdx;

        // Set the smallest child to current node
        PlayerIt smallest = current;

        //Find the smallest among current, left child, and right child
        if (leftChildIdx < heapSize && *leftChild < *smallest) {
            smallest = leftChild;
        }
        if (rightChildIdx < heapSize && *rightChild < *smallest) {
            smallest = rightChild;
        }

        // If the smallest is still current, we're done
        if (smallest == current) {
            break;
        }

        // Swap current with smallest child
        std::swap(*current, *smallest);
        current = smallest;
    }
}

/**
 * @brief Exhausts a stream of Players (ie. until there are none left) such that we:
 * 1) Maintain a running collection of the <reporting_interval> highest leveled players
 * 2) Record the Player level after reading every <reporting_interval> players
 *    representing the minimum level required to be in the leaderboard at that point.
 *
 * @note You should use NOT use a priority-queue.
 *       Instead, use a vector, the STL heap operations, & `replaceMin()`
 *
 * @param stream A stream providing Player objects
 * @param reporting_interval The frequency at which to record cutoff levels
 * @return A RankingResult in which:
 * - top_       -> Contains the top <reporting_interval> Players read in the stream in
 *                 sorted (least to greatest) order
 * - cutoffs_   -> Maps player count milestones to minimum level required at that point
 *                 including the minimum level after ALL players have been read, regardless
 *                 of being a multiple of the reporting interval
 * - elapsed_   -> Contains the duration (ms) of the selection/sorting operation
 *                 excluding fetching the next player in the stream
 *
 * @post All elements of the stream are read until there are none remaining.
 *
 * @example Suppose we have:
 * 1) A stream with 132 players
 * 2) A reporting interval of 50
 *
 * Then our resulting RankingResult might contain something like:
 * top_ = { Player("RECLUSE", 994), Player("WYLDER", 1002), ..., Player("DUCHESS", 1399) }, with length 50
 * cutoffs_ = { 50: 239, 100: 992, 132: 994 } (see RankingResult explanation)
 * elapsed_ = 0.003 (Your runtime will vary based on hardware)
 */
RankingResult rankIncoming(PlayerStream& stream, const size_t& reporting_interval) {
    //Start timer for elapsed_
    auto start = std::chrono::high_resolution_clock::now();

    //Creates a vector to store the top players and a map for cutoffs
    std::vector<Player> topPlayers;
    std::unordered_map<size_t, size_t> cutoffs;
    size_t playerCount = 0;

    //Process the stream until no players remain
    while (stream.remaining() > 0) {
        Player currentPlayer = stream.nextPlayer();
        playerCount++;

        if (topPlayers.size() < reporting_interval) {
            topPlayers.push_back(currentPlayer);
            if (topPlayers.size() == reporting_interval) {
                std::make_heap(topPlayers.begin(), topPlayers.end(), std::greater<Player>());
            }
        } else if (currentPlayer > topPlayers.front()) {
            replaceMin(topPlayers.begin(), topPlayers.end(), currentPlayer);
        }

        if (playerCount % reporting_interval == 0) {
            cutoffs[playerCount] = topPlayers.front().level_;
        }
    }

    // Record cutoff for total players if not already recorded
    if (cutoffs.find(playerCount) == cutoffs.end()) {
        cutoffs[playerCount] = topPlayers.front().level_;
    }

    //Sort the top players in ascending order
    std::sort(topPlayers.begin(), topPlayers.end());

    //Stop timer and calculate elapsed_
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    //Return the Ranking Result object
    return RankingResult(topPlayers, cutoffs, elapsed);
}
};