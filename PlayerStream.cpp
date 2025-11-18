#include "PlayerStream.hpp"

/**
* @brief Constructs a VectorPlayerStream from a vector of Players.
*
* Initializes the stream with a sequence of Player objects matching the
* contents of the given vector.
*
* @param players The vector of Player objects to stream.
*/
VectorPlayerStream::VectorPlayerStream(const std::vector<Player>& players): players_(players), index_(0) {}

/**
* @brief Retrieves the next Player in the stream.
*
* @return The next Player object in the sequence.
* @post Updates members so a subsequent call to nextPlayer() yields the Player
* following that which is returned.

* @throws std::runtime_error If there are no more players remaining in the stream.
*/
Player VectorPlayerStream::nextPlayer() {
    if (index_ >= players_.size()) { // Index is greater than or equal to number of players. No more players to fetch
        throw std::runtime_error("No more players to fetch");
    }

    return players_[index_++]; // Return the current player and increment index
}

/**
* @brief Returns the number of players remaining in the stream.
*
* @return The count of players left to be read.
*/
size_t VectorPlayerStream::remaining() const { // see how many instances remaining to be fetched
    return players_.size() - index_; // Returns the number of players left to be read
}
