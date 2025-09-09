import { writable } from 'svelte/store'

function createGameStore() {
    const state = writable({ lobbies: [] })

    const playerColors = ['#ef4444', '#3b82f6', '#22c55e', '#eab308', '#8b5cf6', '#ec4899', '#f97316', '#14b8a6']

    const setFromMessage = (msg) => {
        state.update((s) => {
            switch (msg.type) {
                case 'lobby_list':
                    return { ...s, lobbies: msg.lobbies || [] }
                case 'lobby_created': {
                    const l = msg.lobby
                    const exists = s.lobbies.find((x) => x.id === l.id)
                    return { ...s, lobbies: exists ? s.lobbies : [l, ...s.lobbies] }
                }
                case 'lobby_update': {
                    const l = msg.lobby
                    const lobbies = s.lobbies.some((x) => x.id === l.id)
                        ? s.lobbies.map((x) => (x.id === l.id ? l : x))
                        : [l, ...s.lobbies]
                    const currentLobby = s.currentLobby && s.currentLobby.id === l.id ? l : s.currentLobby
                    return { ...s, lobbies, currentLobby }
                }
                case 'lobby_gone': {
                    const lobbies = s.lobbies.filter((x) => x.id !== msg.lobby_id)
                    const currentLobby = s.currentLobby && s.currentLobby.id === msg.lobby_id ? undefined : s.currentLobby
                    return { ...s, lobbies, currentLobby }
                }
                case 'joined': {
                    const currentLobby = s.lobbies.find((l) => l.id === msg.lobby_id) || s.currentLobby
                    return { ...s, currentLobby }
                }
                case 'left': {
                    return { ...s, currentLobby: undefined }
                }
                case 'game_state': {
                    const g = { ...msg }
                    if (Array.isArray(g.alive_players) && g.alive_players.length <= 1) {
                        g.winner = g.alive_players[0] || null
                    }
                    if (Array.isArray(g.move_history)) {
                        g.lastMove = g.move_history[g.move_history.length - 1] || null
                    }
                    g.ourTurn = g.current_player === s.playerId
                    return { ...s, game: g }
                }
            }
            return s
        })
    }

    return { state: { subscribe: state.subscribe }, setFromMessage, playerColors }
}

export const gameStore = createGameStore()
