<script>
    import { gameStore } from "../stores/gameStore.js";
    const gState = gameStore.state;
    $: game = $gState.game;
</script>

<div class="space-y-3">
    <h3 class="font-semibold">Players</h3>
    <ul class="space-y-1">
        {#each game?.players || [] as p}
            <li class="flex items-center gap-2 text-sm">
                <span
                    class="inline-block w-3 h-3 rounded-full"
                    style="background:{p.color || 'gray'}"
                ></span>
                <span class="font-mono">{p.id}</span>
                {#if p.eliminated}
                    <span class="text-red-400 ml-auto">eliminated</span>
                {/if}
                {#if game?.activePlayerId === p.id}
                    <span class="ml-auto text-indigo-400">your turn</span>
                {/if}
            </li>
        {/each}
    </ul>
</div>
