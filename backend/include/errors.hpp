#pragma once

#include <stdexcept>
namespace errors {
const std::logic_error kLobbyNotFound("Lobby not found");
const std::logic_error kPlayerNotFound("Player not found");
const std::logic_error kPlayerAlreadyInLobby("Player is already in the lobby");
const std::logic_error kPlayerNotInLobby("Player is not in the lobby");
const std::logic_error kPlayerNotInGame("Player is not in the game");
const std::logic_error kNotLobbyHost("Player is not the lobby host");
const std::logic_error kNotEnoughPlayers(
    "Not enough players to start the game");
const std::logic_error kLobbyFull("Lobby is full");
const std::logic_error kGameAlreadyStarted("Game has already started");
}  // namespace errors
