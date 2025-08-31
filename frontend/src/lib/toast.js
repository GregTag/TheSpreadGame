import { writable } from 'svelte/store'

export const toasts = writable([])

export function showToast(text, timeout = 3000) {
    const id = Math.random().toString(36).slice(2)
    toasts.update((t) => [...t, { id, text }])
    setTimeout(() => {
        toasts.update((t) => t.filter((x) => x.id !== id))
    }, timeout)
}
