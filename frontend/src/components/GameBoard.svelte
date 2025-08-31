<script>
    import { gameStore } from "../stores/gameStore.js";
    import { wsStore } from "../stores/WebSocketStore.js";

    const gState = gameStore.state;
    $: g = $gState.game;

    function clickCell(index) {
        wsStore.send({ type: "make_move", square_id: index });
    }

    $: width = g?.board?.width || 8;
    $: height = g?.board?.height || 8;
    $: cells =
        g?.board?.cells ||
        Array.from({ length: width * height }, (_, i) => ({
            id: i,
            owner: null,
            count: 0,
        }));

    let prev = [];
    let changed = new Set();
    $: {
        const newChanged = new Set();
        if (prev.length === cells.length) {
            for (let i = 0; i < cells.length; i++) {
                const a = prev[i];
                const b = cells[i];
                if (!a || !b) continue;
                if (a.count !== b.count || a.owner !== b.owner)
                    newChanged.add(i);
            }
        }
        changed = newChanged;
        prev = cells.map((c) => ({ id: c.id, owner: c.owner, count: c.count }));
    }

    const ownerColor = (owner) =>
        owner ? gameStore.playerColorFor(owner) : "transparent";
</script>

<div
    class="grid gap-1"
    style={`grid-template-columns: repeat(${width}, minmax(0,1fr));`}
>
    {#each cells as c, i}
        <button
            type="button"
            class="relative aspect-square border border-slate-700 rounded-sm overflow-hidden cursor-pointer focus:outline-none focus:ring-2 focus:ring-indigo-400 transition-colors"
            style={`background:${ownerColor(c.owner)}${changed.has(i) ? "55" : "22"}`}
            on:click={() => clickCell(i)}
        >
            <!-- dots -->
            <div class="absolute inset-0">
                <div
                    class={`absolute left-1/2 -translate-x-1/2 top-1 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                    style={`background:${c.count > 0 ? "white" : ""}`}
                ></div>
                <div
                    class={`absolute right-1 top-1/2 -translate-y-1/2 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                    style={`background:${c.count > 1 ? "white" : ""}`}
                ></div>
                <div
                    class={`absolute left-1/2 -translate-x-1/2 bottom-1 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                    style={`background:${c.count > 2 ? "white" : ""}`}
                ></div>
                <div
                    class={`absolute left-1 top-1/2 -translate-y-1/2 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                    style={`background:${c.count > 3 ? "white" : ""}`}
                ></div>
            </div>
        </button>
    {/each}
</div>
