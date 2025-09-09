<script>
    import { onMount } from "svelte";
    import LobbyList from "./components/LobbyList.svelte";
    import LobbyView from "./components/LobbyView.svelte";
    import GameBoard from "./components/GameBoard.svelte";
    import PlayerPanel from "./components/PlayerPanel.svelte";
    import Toast from "./components/Toast.svelte";
    import { showToast } from "./lib/toast.js";
    import { wsStore } from "./stores/WebSocketStore.js";
    import { gameStore } from "./stores/gameStore.js";

    const wsState = wsStore.state;
    const gState = gameStore.state;

    onMount(() => {
        wsStore.connect();
        const unsub = wsStore.subscribeMessages((msg) => {
            gameStore.setFromMessage(msg);
            if (msg.type === "error" && msg.message) {
                showToast(msg.message);
                console.error("Error from server:", msg.message);
            }
        });
        return () => unsub();
    });

    $: status = $wsState.status;
    $: currentLobby = $gState.currentLobby;
    $: game = $gState.game;
</script>

<main class="max-w-6xl mx-auto p-4 space-y-4">
    <Toast />
    <header class="flex items-center justify-between">
        <h1 class="text-2xl font-bold">Spread</h1>
        <div class="text-sm text-slate-400">WS: {status}</div>
    </header>

    {#if !currentLobby && !game}
        <LobbyList />
    {/if}

    {#if currentLobby && !game}
        <LobbyView />
    {/if}

    {#if game}
        <section
            class="grid grid-cols-1 md:grid-cols-[1fr_280px] gap-4 items-start"
        >
            <div
                class="bg-slate-800/60 border border-slate-700 rounded-lg shadow p-3"
            >
                <GameBoard />
            </div>
            <div
                class="bg-slate-800/60 border border-slate-700 rounded-lg shadow p-3"
            >
                <PlayerPanel />
            </div>
        </section>
        {#if game?.winner}
            <div
                class="fixed inset-0 bg-black/60 flex items-center justify-center z-50"
            >
                <div
                    class="bg-slate-800 border border-slate-700 rounded-lg shadow p-6 w-80 text-center space-y-3"
                >
                    <h3 class="text-lg font-semibold">Game Over</h3>
                    <p class="text-slate-300">
                        Winner: <span class="font-mono">{game.winner}</span>
                    </p>
                </div>
            </div>
        {/if}
    {/if}
</main>
