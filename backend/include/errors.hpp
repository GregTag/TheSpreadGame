#pragma once

#include <stdexcept>
namespace errors {
const std::logic_error LobbyNotFound("Lobby not found");
const std::logic_error PlayerNotFound("Player not found");
const std::logic_error PlayerAlreadyInLobby("Player is already in the lobby");
const std::logic_error PlayerNotInLobby("Player is not in the lobby");
const std::logic_error LobbyFull("Lobby is full");
const std::logic_error GameAlreadyStarted("Game has already started");
}  // namespace errors
