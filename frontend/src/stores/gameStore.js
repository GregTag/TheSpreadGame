import { writable } from 'svelte/store'

function createGameStore() {
    const state = writable({ lobbies: [] })

    const playerColors = ['#ef4444', '#3b82f6', '#22c55e', '#eab308']

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
                    if (Array.isArray(g.players)) {
                        g.players = g.players.map((p) => ({ ...p, color: p.color || playerColorFor(p.id || p.player_id || '') }))
                    }
                    if (g.board && !g.board.cells && Array.isArray(g.board.squares)) {
                        // normalize to cells
                        g.board.cells = g.board.squares.map((sq, i) => ({ id: i, owner: sq.owner_id || sq.owner || null, count: sq.dots || sq.count || 0 }))
                    }
                    return { ...s, game: g }
                }
                case 'move_result': {
                    const merged = { ...(s.game || {}), ...(msg.state || {}) }
                    if (Array.isArray(merged.players)) {
                        merged.players = merged.players.map((p) => ({ ...p, color: p.color || playerColorFor(p.id || p.player_id || '') }))
                    }
                    if (merged.board && !merged.board.cells && Array.isArray(merged.board.squares)) {
                        merged.board.cells = merged.board.squares.map((sq, i) => ({ id: i, owner: sq.owner_id || sq.owner || null, count: sq.dots || sq.count || 0 }))
                    }
                    return { ...s, game: merged }
                }
                case 'player_eliminated': {
                    const game = s.game ? { ...s.game } : undefined
                    if (game && game.players) {
                        const p = game.players.find((p) => p.id === msg.player_id)
                        if (p) p.eliminated = true
                    }
                    return { ...s, game }
                }
                case 'game_over': {
                    const game = s.game ? { ...s.game, winner: msg.winner } : undefined
                    return { ...s, game }
                }
            }
            return s
        })
    }

    const playerColorFor = (pid) => {
        const idx = Math.abs([...pid].reduce((a, c) => a + c.charCodeAt(0), 0)) % playerColors.length
        return playerColors[idx]
    }

    return { state: { subscribe: state.subscribe }, setFromMessage, playerColorFor }
}

export const gameStore = createGameStore()
