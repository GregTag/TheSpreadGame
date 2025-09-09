<script>
    import { gameStore } from "../stores/gameStore.js";
    const gState = gameStore.state;
    $: game = $gState.game;
</script>

<div class="space-y-3">
    <h3 class="font-semibold">Players</h3>
    <ul class="space-y-1">
        {#each game?.alive_players || [] as pid, idx}
            <li class="flex items-center gap-2 text-sm">
                <span
                    class="inline-block w-3 h-3 rounded-full"
                    style="background:{gameStore.playerColors[idx]}"
                ></span>
                <span
                    class="font-mono"
                    class:underline={game?.current_player === pid}>{pid}</span
                >
            </li>
        {/each}
    </ul>
</div>
