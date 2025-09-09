<script>
    import { gameStore } from "../stores/gameStore.js";
    import { wsStore } from "../stores/WebSocketStore.js";

    const TOP = 1,
        RIGHT = 2,
        BOTTOM = 4,
        LEFT = 8;

    const gState = gameStore.state;
    $: g = $gState.game;

    function clickCell(index) {
        wsStore.send({ type: "make_move", cell_idx: index });
    }

    $: width = g.field.width;
    // $: height = g.field.height;
    $: cells = g.field.cells;

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

    function sumBits(n) {
        let count = 0;
        for (let i = 0; i < 4; i++) {
            if (n & (1 << i)) count++;
        }
        return count;
    }
    function ownerColor(idx) {
        return idx > 0 ? gameStore.playerColors[idx - 1] : "transparent";
    }
</script>

<div
    class="grid gap-1"
    style={`grid-template-columns: repeat(${width}, minmax(0,1fr));`}
>
    {#each cells as c, i}
        <button
            type="button"
            class="relative aspect-square border border-slate-700 rounded-sm overflow-hidden cursor-pointer focus:outline-none focus:ring-2 focus:ring-indigo-400 transition-colors"
            style={`background:${ownerColor(c.owner_index)}${changed.has(i) ? "55" : "22"}`}
            on:click={() => clickCell(i)}
        >
            <!-- dots -->
            <div class="absolute inset-0">
                {#if c.configuration & TOP}
                    <div
                        class={`absolute left-1/2 -translate-x-1/2 top-1 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                        style={`background:${c.fullness > 0 ? "white" : ""}`}
                    ></div>
                {/if}
                {#if c.configuration & RIGHT}
                    <div
                        class={`absolute right-1 top-1/2 -translate-y-1/2 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                        style={`background:${c.fullness > sumBits(c.configuration % RIGHT) ? "white" : ""}`}
                    ></div>
                {/if}
                {#if c.configuration & BOTTOM}
                    <div
                        class={`absolute left-1/2 -translate-x-1/2 bottom-1 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                        style={`background:${c.fullness > sumBits(c.configuration % BOTTOM) ? "white" : ""}`}
                    ></div>
                {/if}
                {#if c.configuration & LEFT}
                    <div
                        class={`absolute left-1 top-1/2 -translate-y-1/2 w-2 h-2 rounded-full border border-white/60 ${changed.has(i) ? "animate-pulse" : ""}`}
                        style={`background:${c.fullness > sumBits(c.configuration % LEFT) ? "white" : ""}`}
                    ></div>
                {/if}
            </div>
        </button>
    {/each}
</div>
