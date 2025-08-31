<script>
    import { wsStore } from "../stores/WebSocketStore.js";
    import { gameStore } from "../stores/gameStore.js";

    const wsState = wsStore.state;
    const gState = gameStore.state;

    let creating = false;

    $: lobbies = $gState.lobbies;
    $: status = $wsState.status;

    function refresh() {
        wsStore.send({ type: "list_lobbies" });
    }

    function createLobby() {
        if (creating) return;
        creating = true;
        wsStore.send({
            type: "create_lobby",
            name: "Quick Lobby",
            board_size: [8, 8],
            max_players: 4,
        });
        setTimeout(() => (creating = false), 500);
    }

    function join(lobbyId) {
        wsStore.send({ type: "join_lobby", lobby_id: lobbyId });
    }
</script>

<div class="space-y-4">
    <div class="flex items-center justify-between">
        <h2 class="text-xl font-semibold">Lobbies</h2>
        <div class="flex gap-2">
            <button
                class="px-3 py-1.5 rounded-md bg-slate-700 hover:bg-slate-600 disabled:opacity-50"
                on:click={refresh}
                disabled={status !== "open"}>Refresh</button
            >
            <button
                class="px-3 py-1.5 rounded-md bg-indigo-600 hover:bg-indigo-500 disabled:opacity-50"
                on:click={createLobby}
                disabled={status !== "open" || creating}
            >
                {creating ? "Creating…" : "Create Lobby"}
            </button>
        </div>
    </div>

    <div
        class="bg-slate-800/60 border border-slate-700 rounded-lg shadow overflow-hidden"
    >
        <table class="w-full text-sm">
            <thead class="bg-slate-800/80">
                <tr>
                    <th class="text-left px-3 py-2">ID</th>
                    <th class="text-left px-3 py-2">Name</th>
                    <th class="text-left px-3 py-2">Players</th>
                    <th class="text-left px-3 py-2">Size</th>
                    <th class="text-left px-3 py-2">Action</th>
                </tr>
            </thead>
            <tbody>
                {#if lobbies.length === 0}
                    <tr
                        ><td colspan="5" class="px-3 py-3 text-slate-400"
                            >No lobbies yet.</td
                        ></tr
                    >
                {:else}
                    {#each lobbies as l}
                        <tr class="border-t border-slate-700">
                            <td class="px-3 py-2 font-mono">{l.id}</td>
                            <td class="px-3 py-2"
                                >{l.options?.name || "Lobby"}</td
                            >
                            <td class="px-3 py-2"
                                >{l.players?.length || 0}/{l.options
                                    ?.max_players || 4}</td
                            >
                            <td class="px-3 py-2"
                                >{l.options?.width || 8}×{l.options?.height ||
                                    8}</td
                            >
                            <td class="px-3 py-2">
                                <button
                                    class="px-3 py-1 rounded-md bg-slate-700 hover:bg-slate-600"
                                    on:click={() => join(l.id)}>Join</button
                                >
                            </td>
                        </tr>
                    {/each}
                {/if}
            </tbody>
        </table>
    </div>
</div>
