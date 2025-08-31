import { writable } from 'svelte/store'

function createWebSocketStore() {
    const state = writable({ status: 'connecting' })
    let ws = null

    const send = (msg) => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify(msg))
        }
    }

    const connect = () => {
        const url = (location.protocol === 'https:' ? 'wss://' : 'ws://') + (location.hostname || 'localhost') + ':8080'
        ws = new WebSocket(url)
        state.set({ status: 'connecting' })

        ws.onopen = () => {
            state.update((s) => ({ ...s, status: 'open' }))
            send({ type: 'list_lobbies' })
        }

        ws.onmessage = (ev) => {
            try {
                const data = JSON.parse(ev.data)
                if (data.type === 'server_ready') {
                    state.update((s) => ({ ...s, playerId: data.player_id }))
                }
                listeners.forEach((cb) => cb(data))
            } catch (e) {
                console.warn('Non-JSON message', ev.data)
            }
        }

        ws.onclose = () => state.set({ status: 'closed' })
        ws.onerror = () => state.set({ status: 'error', lastError: 'ws error' })
    }

    const listeners = new Set()
    const subscribeMessages = (cb) => {
        listeners.add(cb)
        return () => listeners.delete(cb)
    }

    return { state: { subscribe: state.subscribe }, connect, send, subscribeMessages }
}

export const wsStore = createWebSocketStore()
