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