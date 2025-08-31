<script>
    import { wsStore } from "../stores/WebSocketStore.js";
    import { gameStore } from "../stores/gameStore.js";

    const gState = gameStore.state;
    const wsState = wsStore.state;

    $: lobby = $gState.currentLobby;
    $: playerId = $wsState.playerId;
    $: isHost = lobby && lobby.host_player_id === playerId;

    function leave() {
        if (!lobby) return;
        wsStore.send({ type: "leave_lobby", lobby_id: lobby.id });
    }
    function start() {
        if (!lobby) return;
        wsStore.send({ type: "start_game", lobby_id: lobby.id });
    }
</script>

<div class="space-y-4">
    <div class="flex items-center justify-between">
        <div>
            <h2 class="text-xl font-semibold">
                Lobby {lobby?.options?.name}
                <span class="text-slate-400">({lobby?.id})</span>
            </h2>
            <div class="text-sm text-slate-400">
                Host: {lobby?.host_player_id}
            </div>
        </div>
        <div class="flex gap-2">
            <button
                class="px-3 py-1.5 rounded-md bg-slate-700 hover:bg-slate-600"
                on:click={leave}>Leave</button
            >
            {#if isHost}
                <button
                    class="px-3 py-1.5 rounded-md bg-indigo-600 hover:bg-indigo-500"
                    on:click={start}>Start Game</button
                >
            {/if}
        </div>
    </div>

    <div class="bg-slate-800/60 border border-slate-700 rounded-lg shadow p-3">
        <h3 class="font-semibold mb-2">Players</h3>
        <ul class="space-y-1">
            {#each lobby?.players || [] as p}
                <li class="text-sm">{p}</li>
            {/each}
        </ul>
    </div>
</div>
